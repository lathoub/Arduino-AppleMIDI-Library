#pragma once

#include "AppleMIDI_Namespace.h"
BEGIN_APPLEMIDI_NAMESPACE

#ifdef APPLEMIDI_DEBUG
namespace {
static void AM_DBG_SETUP(unsigned long baud) {
  APPLEMIDI_DEBUG.begin(baud);
  while (!APPLEMIDI_DEBUG);
}

template <typename T>
static void AM_DBG_PLAIN(T last) {
  APPLEMIDI_DEBUG.println(last);
}

template <typename T, typename... Args>
static void AM_DBG_PLAIN(T head, Args... tail) {
  APPLEMIDI_DEBUG.print(head);
  APPLEMIDI_DEBUG.print(' ');
  AM_DBG_PLAIN(tail...);
}

template <typename... Args>
static void AM_DBG(Args... args) {
  AM_DBG_PLAIN(args...);
}
}  // namespace
#else
#define AM_DBG_SETUP(...)
#define AM_DBG_PLAIN(...)
#define AM_DBG(...)
#endif

END_APPLEMIDI_NAMESPACE
