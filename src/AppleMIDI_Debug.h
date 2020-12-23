#ifdef APPLEMIDI_DEBUG
namespace {
static void DBG_SETUP(unsigned long baud) {
  APPLEMIDI_DEBUG.begin(baud);
  #ifdef __AVR_ATmega32U4__
  while (!APPLEMIDI_DEBUG);
  #endif
}

template <typename T>
static void DBG_PLAIN(T last) {
  APPLEMIDI_DEBUG.println(last);
}

template <typename T, typename... Args>
static void DBG_PLAIN(T head, Args... tail) {
  APPLEMIDI_DEBUG.print(head);
  APPLEMIDI_DEBUG.print(' ');
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