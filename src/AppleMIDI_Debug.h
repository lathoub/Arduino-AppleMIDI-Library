#ifdef DEBUG

namespace {
template <typename T>
static void AM_DBG_PLAIN(T last) {
  DEBUG.println(last);
}

template <typename T, typename... Args>
static void AM_DBG_PLAIN(T head, Args... tail) {
  DEBUG.print(head);
  DEBUG.print(' ');
  DBG_PLAIN(tail...);
}

template <typename... Args>
static void AM_DBG(Args... args) {
  DBG_PLAIN(args...);
}
}  // namespace

#else
#define AM_DBG_PLAIN(...)
#define AM_DBG(...)

#endif
