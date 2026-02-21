#pragma once

#ifdef SerialMon
static inline void AM_DBG_SETUP(unsigned long baud) {
  SerialMon.begin(baud);
  const unsigned long timeout_ms = 2000;
  const unsigned long start_ms = millis();
  while (!SerialMon && (millis() - start_ms) < timeout_ms) {
    // Avoid hard lock on boards without native USB or no host.
  }
}

template <typename T>
static inline void AM_DBG_PLAIN(T last) {
  SerialMon.println(last);
}

template <typename T, typename... Args>
static inline void AM_DBG_PLAIN(T head, Args... tail) {
  SerialMon.print(head);
  SerialMon.print(' ');
  AM_DBG_PLAIN(tail...);
}

template <typename... Args>
static inline void AM_DBG(Args... args) {
  AM_DBG_PLAIN(args...);
}
#else
#define AM_DBG_SETUP(...)
#define AM_DBG_PLAIN(...)
#define AM_DBG(...)
#endif
