#ifndef BSNET_UTILITY_HPP
#define BSNET_UTILITY_HPP

#include <chrono>
#include <functional>
#include <stdexcept>
#include <string>

namespace bsnet {

using Duration = std::chrono::milliseconds;

#define DECL_ERR(clsname)                                                      \
  struct clsname : std::runtime_error {                                        \
    clsname();                                                                 \
    clsname(const std::string &msg);                                           \
  }

#define DECL_MSG_ERR(clsname)                                                  \
  struct clsname : std::runtime_error {                                        \
    clsname(const std::string &msg);                                           \
  }

#define IMPL_ERR(clsname)                                                      \
  clsname::clsname() : std::runtime_error(::strerror(errno)) {}                \
  clsname::clsname(const std::string &msg) : std::runtime_error(msg) {}

#define IMPL_MSG_ERR(clsname)                                                  \
  clsname::clsname(const std::string &msg)                                     \
      : std::runtime_error(msg + ", " + ::strerror(errno)) {}

#define CHECKED_TCPOP(expr)                                                    \
  if (!(expr)) {                                                               \
    throw tcp_error(#expr);                                                    \
  }

#define CHECKED(expr, errcls)                                                  \
  if (!(expr)) {                                                               \
    throw errcls(#expr);                                                       \
  }

class NonCopyable {
public:
  NonCopyable() = default;
  ~NonCopyable() = default;
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable &operator=(const NonCopyable &) = delete;
};

} // namespace bsnet

#endif // BSNET_UTILITY_HPP