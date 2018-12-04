#include "boundedthreadedreceiver.h"

void *BoundedThreadedReceiver::deliverLoop(void *arg)
{
    BoundedThreadedReceiver* receiver = (BoundedThreadedReceiver*) arg;

    while(true)
    {
        // waiting for data
        sem_wait(&(receiver->fill_sem));

        // locking the buffer
        receiver->m.lock();

        // found something to deliver?
        bool found = false;

        // buffer
        queued_message msg;

        // loop over queue
        if(!receiver->messages.empty())
        {
            found = true;

            // copying data
            msg = *(receiver->messages.begin());

            // removing first element
            receiver->messages.pop_front();

            // allowing for one more message in the buffer
            sem_post(&receiver->empty_sem);
        }

        // now data is in msg, so can unlock the buffer
        receiver->m.unlock();

        // delivering messages, if any
        if(found)
        {
            vector<Target*>::iterator it;
            if(msg.logical_source == 0) for(it = receiver->targets.begin(); it != receiver->targets.end(); it++)
                (*it)->onMessage(msg.source, msg.data.c_str(), msg.data.length());
            else for(it = receiver->targets.begin(); it != receiver->targets.end(); it++)
                (*it)->onMessage(msg.source, msg.logical_source, msg.data.c_str(), msg.data.length());
        }
    }

    // never returns
    return nullptr;
}

void BoundedThreadedReceiver::deliverToAll(unsigned source, unsigned logical_source, const char* message, unsigned length)
{
    // taking one from empty semaphore
    sem_wait(&empty_sem);

    m.lock();

    // adding message to the buffer
    queued_message buf;
    buf.data = string(message, length);
    buf.source = source;
    buf.logical_source = logical_source;
    messages.push_back(buf);

    m.unlock();

    // +1 to the semaphore
    sem_post(&fill_sem);
}

void BoundedThreadedReceiver::deliverToAll(unsigned source, const char* message, unsigned length)
{
    // deliver to all with three arguments
    deliverToAll(source, 0, message, length);
}

BoundedThreadedReceiver::BoundedThreadedReceiver(unsigned this_process, unsigned buffer_size, Target *target) : Receiver(this_process, target)
{
    // creating the fill semaphore
    sem_init(&fill_sem, 0, 0);
    sem_init(&empty_sem, 0, buffer_size);

    // creating the delivery thread
    pthread_create(&deliver_thread, nullptr, &BoundedThreadedReceiver::deliverLoop, this);
}

bool BoundedThreadedReceiver::canDeliver()
{
    // value of the semaphore EMPTY
    int val;
    sem_getvalue(&empty_sem, &val);

    // can deliver <=> there is at least one value to take from the semaphore
    return val >= 1;
}
