/** @file common.h
 * @brief Int to chars conversion
 * @author Volodin Sergei, EPFL
 * @date October 2018
 */

#ifndef COMMON_H
#define COMMON_H

#include <inttypes.h>
#include "inmemorylog.h"
#include <sstream>
#include <chrono>

using std::stringstream;
using std::chrono::steady_clock;

/** @macro minimal of two numbers */
#define min(x, y) ((x) > (y) ? (y) : (x))

/** @macro minimal of two numbers */
#define max(x, y) ((x) < (y) ? (y) : (x))

/** @macro Print messages stored in memory */
//#define INMEMORY_PRINT 1

/// @macro Dump to a separate file immediately
//#define IMMEDIATE_FILE 1

#define TIME_MS_NOW() (std::chrono::duration_cast<std::chrono::milliseconds>(steady_clock::now().time_since_epoch()).count())

/**
 * @brief int32ToChars Convert unsigned int32 to chars
 * @param input int
 * @param dest chars
 */
void int32ToChars(uint32_t input, char* dest);

/**
 * @brief charsToInt32 Convert chars to uint32
 * @param input chars
 * @return uint32
 */
uint32_t charsToInt32(char* input);

/** @brief Maximal buffer size */
const int MAXLEN = 1000;

/**
 * @brief memorylog In Memory Log
 */
extern InMemoryLog* memorylog;

/// @macro Delay after each UDPSender::send
//#define UDPSENDER_DELAY_MS 0.5

#endif // COMMON_H
