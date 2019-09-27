#ifndef PROXY_HPP
#define PROXY_HPP

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

#include "tun_adapter.hpp"
using namespace my_tun;

namespace my_proxy
{
  class ProxyServer : public std::enable_shared_from_this<ProxyServer>
  {
  public:
    ProxyServer(boost::asio::io_service& io_service,
                const std::string& fwd_host,
                unsigned short fwd_port);

  private:
    void tun_read_done(const boost::system::error_code& ec, size_t cnt);
    void tun_write_done(const boost::system::error_code& ec, size_t cnt);

    void remote_read_done(const boost::system::error_code& ec, size_t cnt);
    void remote_write_done(const boost::system::error_code& ec, size_t cnt);

    //------------------------------------------------------------------------------
    tun_adapter tun_;

    tcp::socket in_socket_;

    tcp::socket out_socket_;

    std::string remote_host_;
    unsigned short remote_port_;

    size_t buffer_size_;
  };
}

#endif
