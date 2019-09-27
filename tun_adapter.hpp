#ifndef TUN_ADAPTER_HPP
#define TUN_ADAPTER_HPP

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <iostream>
#include <map>
#include <string>

#include <sys/types.h>
#include <sys/wait.h>
#include <ifaddrs.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "trace.hpp"

#include "fd_handler.hpp"
using namespace my_fd;

namespace my_tun
{
  class tun_adapter
  {
    boost::asio::posix::stream_descriptor m_descriptor;
    std::string m_name;

    fd_handler open_device(const std::string& name);
    fd_handler open_socket(int family);

  public:
    tun_adapter(boost::asio::io_service& _io_service)
        : m_descriptor(_io_service)
        , m_name()
    {
      MYLOG2();
    }
    tun_adapter(const tun_adapter&) = delete;
    tun_adapter& operator=(const tun_adapter&) = delete;

    ~tun_adapter()
    {
      MYLOG2();
    }

    //---------------------------------------------------------------------
    // READ operations
    //---------------------------------------------------------------------
    template <typename MutableBufferSequence, typename ReadHandler>
    void async_read(const MutableBufferSequence& buffers, ReadHandler handler)
    {
      MYLOG2();
      m_descriptor.async_read_some(buffers, handler);
    }

    template <typename MutableBufferSequence>
    size_t read(const MutableBufferSequence& buffers)
    {
      return m_descriptor.read_some(buffers);
    }

    template <typename MutableBufferSequence>
    size_t read(const MutableBufferSequence& buffers, boost::system::error_code& ec)
    {
      return m_descriptor.read_some(buffers, ec);
    }

    //---------------------------------------------------------------------
    // WRITE operations
    //---------------------------------------------------------------------
    template <typename ConstBufferSequence, typename WriteHandler>
    void async_write(const ConstBufferSequence& buffers, WriteHandler handler)
    {
      m_descriptor.async_write_some(buffers, handler);
    }

    template <typename ConstBufferSequence>
    size_t write(const ConstBufferSequence& buffers)
    {
      return m_descriptor.write_some(buffers);
    }

    template <typename ConstBufferSequence>
    size_t write(const ConstBufferSequence& buffers, boost::system::error_code& ec)
    {
      return m_descriptor.write_some(buffers, ec);
    }

    //---------------------------------------------------------------------
    // OPEN operations
    //---------------------------------------------------------------------
    void open(const std::string& name = "");

    //---------------------------------------------------------------------
    // MISC operations
    //---------------------------------------------------------------------
    void configure();

    void set_connected_state(bool connected);

    void cancel()
    {
      m_descriptor.cancel();
    }

    void cancel(boost::system::error_code& ec)
    {
      m_descriptor.cancel(ec);
    }

    boost::asio::io_service& get_io_service()
    {
      return m_descriptor.get_io_service();
    }

    const std::string& name() const
    {
      return m_name;
    }

    bool is_open() const
    {
      return m_descriptor.is_open();
    }

    void close()
    {
      m_descriptor.close();
    }

    boost::system::error_code close(boost::system::error_code& ec)
    {
      return m_descriptor.close(ec);
    }

    boost::asio::posix::stream_descriptor& descriptor()
    {
      return m_descriptor;
    }
  };
} // namespace my_tun

#endif
