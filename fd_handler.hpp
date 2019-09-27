#ifndef FD_HANDLER_HPP
#define FD_HANDLER_HPP

namespace my_fd
{
  class fd_handler
  {
    int m_fd;

  public:
    fd_handler() : m_fd(-1) {}
    explicit fd_handler(int fd) : m_fd(fd) {}
    fd_handler(const fd_handler &) = delete;
    fd_handler &operator=(const fd_handler &) = delete;
    fd_handler(fd_handler &&other)
    : m_fd(other.m_fd) { other.m_fd = -1; }
    fd_handler &operator=(fd_handler &&other) {
      using std::swap;
      swap(m_fd, other.m_fd);
      return *this;
    }
    ~fd_handler() { if (m_fd >= 0) { ::close(m_fd); }}
    int native_handle() const { return m_fd; }
    bool valid() const { return (m_fd >= 0); }
    int release() {
      const int result = m_fd;
      m_fd = -1;
      return result;
    }
  };
}

#endif
