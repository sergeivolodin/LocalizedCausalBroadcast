//
//  perfectlink.cpp
//  DA_PaymentSystem
//
//  Created by Vincent Ballet on 02/10/2018.
//  Copyright © 2018 Vincent Ballet. All rights reserved.
//

#include "perfectlink.h"
#include "common.h"
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <cassert>

using namespace std;
using chrono::steady_clock;
using std::make_pair;

void PerfectLink::onMessage(unsigned source, char *buf, unsigned len)
{
    assert(len >= 4);
    // receiving an ACK from a sent message
    if(len == 7 && memmem(buf + 4, 3, "ACK", 3))
    {
        int seqnumack = charsToInt32(buf);
        cout << "** Received ACK " << seqnumack << endl;

        mtx.lock();
        free(msgs[seqnumack].second);
        msgs.erase(seqnumack);
        mtx.unlock();

    } // receiving content from another process => we send an ACK
    else {
        int tmp = charsToInt32(buf);
        cout << "** Received content " << tmp << endl;
        if(target)
            target->onMessage(source, buf + 4, len - 4);
        char sdata[7];
        memcpy(sdata, buf, 4);
        memcpy(sdata + 4, "ACK", 3);
        cout << "** Sending ACK for " << tmp << endl;
        s->send(sdata, 7);
    }
}

PerfectLink::PerfectLink(Sender *s, Receiver *r, Target *target) :
    Sender(s->getTarget()), Receiver(r->getThis(), target)
{
    r->setTarget(this);
    this->s = s;
    this->r = r;
    this->seqnum = 0;
}

void PerfectLink::send(char* buffer, int length)
{
    // filling the buffer
    craftAndStoreMsg(buffer, length);

    // Send all messages if ACK missing
    map<int, pair<int, char*> >::iterator it;

    // start of critical section
    mtx.lock();
    for (it = this->msgs.begin(); it != this->msgs.end(); it++)
    {
        // seq number
        int tmp = (*it).first;

        // data
        char* sdata = (*it).second.second;

        // data length
        int len = (*it).second.first;

        // sending message
        s->send(sdata, 4 + len);
    }

    // end of critical section
    mtx.unlock();

    // waiting if there are messages left
    // WARNING: no retransmission after failure
    waitForAcksOrTimeout();
}

void PerfectLink::waitForAcksOrTimeout()
{
    // waiting until all messages are sent
    steady_clock::time_point begin = steady_clock::now();
    while(this->msgs.size() != 0 ) {
        long a = chrono::duration_cast<chrono::microseconds>(steady_clock::now() - begin).count();
        if (a > TIMEOUT) break;

        /// Sleep 1 millisecond
        /// Drastically reduces CPU load (thread sleep instead of busy wait)
        usleep(1000);
    }
}

void PerfectLink::craftAndStoreMsg(char* buffer, int length)
{
    // allocating new memory
    char* data = static_cast<char*>(malloc(length + 4));

    // adding the message to the list
    mtx.lock();

    // adding the sequence number
    int32ToChars(this->seqnum, data);

    // saving the message
    this->msgs[this->seqnum] = make_pair(length, data);

    // IMPORTANT: incrementing the sequence number
    // Otherwise another thread could send a message with SAME
    // sequence number
    this->seqnum++;

    // finished critical section
    mtx.unlock();
}
