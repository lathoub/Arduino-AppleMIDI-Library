#pragma once

#ifdef SerialMon
namespace {
static void DBG_SETUP(unsigned long baud) {
  SerialMon.begin(baud);
  while (!SerialMon);
}

template <typename T>
static void DBG_PLAIN(T last) {
  SerialMon.println(last);
}

template <typename T, typename... Args>
static void DBG_PLAIN(T head, Args... tail) {
  SerialMon.print(head);
  SerialMon.print(' ');
  DBG_PLAIN(tail...);
}

template <typename... Args>
static void DBG(Args... args) {
  DBG_PLAIN(args...);
}
}  // namespace
#else
#define DBG_SETUP(...)
#define DBG_PLAIN(...)
#define DBG(...)
#endif
