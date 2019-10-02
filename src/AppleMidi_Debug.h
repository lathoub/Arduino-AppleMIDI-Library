#pragma once

#ifndef DEBUGSTREAM
#define DEBUGSTREAM Serial
#endif

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_FATAL 1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_NOTICE 4
#define LOG_LEVEL_TRACE 5
#define LOG_LEVEL_VERBOSE 6

#ifndef DEBUG
#define DEBUG LOG_LEVEL_NONE
#endif

#if DEBUG > LOG_LEVEL_NONE
#define DEBUG_BEGIN(SPEED)  \
  DEBUGSTREAM.begin(SPEED); \
  while (!DEBUGSTREAM)      \
    ;                       \
  DEBUGSTREAM.println(F("Booting..."));
#define F_DEBUG_PRINT(...) DEBUGSTREAM.print(__VA_ARGS__)
#define F_DEBUG_PRINTLN(...) DEBUGSTREAM.println(__VA_ARGS__)
#else
#define DEBUG_BEGIN(SPEED)
#define F_DEBUG_PRINT(...)
#define F_DEBUG_PRINTLN(...)
#endif

#if DEBUG >= LOG_LEVEL_ERROR
#define E_DEBUG_PRINT(...) DEBUGSTREAM.print(__VA_ARGS__)
#define E_DEBUG_PRINTLN(...) DEBUGSTREAM.println(__VA_ARGS__)
#else
#define E_DEBUG_PRINT(...)
#define E_DEBUG_PRINTLN(...)
#endif

#if DEBUG >= LOG_LEVEL_WARNING
#define W_DEBUG_PRINT(...) DEBUGSTREAM.print(__VA_ARGS__)
#define W_DEBUG_PRINTLN(...) DEBUGSTREAM.println(__VA_ARGS__)
#else
#define W_DEBUG_PRINT(...)
#define W_DEBUG_PRINTLN(...)
#endif

#if DEBUG >= LOG_LEVEL_NOTICE
#define N_DEBUG_PRINT(...) DEBUGSTREAM.print(__VA_ARGS__)
#define N_DEBUG_PRINTLN(...) DEBUGSTREAM.println(__VA_ARGS__)
#else
#define N_DEBUG_PRINT(...)
#define N_DEBUG_PRINTLN(...)
#endif

#if DEBUG >= LOG_LEVEL_TRACE
#define T_DEBUG_PRINT(...) DEBUGSTREAM.print(__VA_ARGS__)
#define T_DEBUG_PRINTLN(...) DEBUGSTREAM.println(__VA_ARGS__)
#else
#define T_DEBUG_PRINT(...)
#define T_DEBUG_PRINTLN(...)
#endif

#if DEBUG >= LOG_LEVEL_VERBOSE
#define V_DEBUG_PRINT(...) DEBUGSTREAM.print(__VA_ARGS__)
#define V_DEBUG_PRINTLN(...) DEBUGSTREAM.println(__VA_ARGS__)
#else
#define V_DEBUG_PRINT(...)
#define V_DEBUG_PRINTLN(...)
#endif
