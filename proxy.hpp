#ifndef PROXY_HPP
#define PROXY_HPP

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

#include "tun_adapter.hpp"
using namespace my_tun;

namespace my_proxy
{
  class proxy : public std::enable_shared_from_this<proxy>
  {
  public:
    proxy(boost::asio::io_service& io_service,
          const std::string& fwd_host,
          unsigned short fwd_port);
    ~proxy();

  private:
    void do_remote_connect();
    void do_tun_read();
    void do_remote_read();

    void remote_connect_done(const boost::system::error_code& ec);
    void tun_read_done(const boost::system::error_code& ec, size_t cnt);
    void tun_write_done(const boost::system::error_code& ec, size_t cnt);
    void remote_read_done(const boost::system::error_code& ec, size_t cnt);
    void remote_write_done(const boost::system::error_code& ec, size_t cnt);

    void s5_auth();
    void s5_connect();

    void s5_auth_done(const boost::system::error_code& ec, size_t length);

    //------------------------------------------------------------------------------
    tun_adapter tun_;

    tcp::socket out_socket_;

    std::string remote_host_;
    unsigned short remote_port_;

    enum { max_data_length = 8192 }; //8KB
    unsigned char tun_data_[max_data_length];
    unsigned char remote_data_[max_data_length];
  };
}

#endif
