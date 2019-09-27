#include <cstdlib>
#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include <fstream>

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

#include "proxy.hpp"
using namespace my_proxy;

#include "proto_parser.hpp"
using namespace my_proto;

#include "trace.hpp"

#define MY_BUF_SIZE 2048

//================================================================================
//================================================================================
static char my_buf[MY_BUF_SIZE];

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
ProxyServer::ProxyServer(boost::asio::io_service& io_service,
            const std::string& fwd_host,
            unsigned short fwd_port)
: tun_(io_service)
, in_socket_(io_service)
, out_socket_(io_service)
, remote_host_(fwd_host)
, remote_port_(fwd_port)
, buffer_size_(MY_BUF_SIZE)
{
  MYLOG2();

  tun_.open();
  tun_.set_connected_state(true);
  tun_.configure();

  MYMSGS("TUN interface is prepared/ready. Try to connect to remote host: "
                       << remote_host_ << ":" << remote_port_);

  boost::asio::ip::tcp::endpoint
    endpoint(boost::asio::ip::address::from_string(remote_host_), remote_port_);

  out_socket_.async_connect(endpoint,
      [this](const boost::system::error_code& ec)
      {
        if (!ec)
        {
          MYMSGS("Connected to " << remote_host_ << ":" << remote_port_);
          tun_.async_read(boost::asio::buffer(my_buf, sizeof(my_buf)),
                          boost::bind(&ProxyServer::tun_read_done, this, _1, _2));
        }
        else
        {
          MYMSGS("Failed to connect " << remote_host_ << ":" << remote_port_
                              << " Error: " << ec.message());
        }
      });
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void ProxyServer::tun_read_done(const boost::system::error_code& ec, size_t cnt)
{
  if (!ec)
  {
    MYMSGS("TUN read done. Received: " << cnt << " bytes. Now write REMOTE.");

    boost::asio::const_buffer buffer(my_buf, cnt);

    proto_parser::parse_and_print_packet(my_buf);

    boost::asio::async_write(out_socket_, boost::asio::buffer(buffer),
        boost::bind(&ProxyServer::remote_write_done, this, _1, _2));
  }
  else
  {
    MYMSGS("TUN read done. Received: " << cnt << " bytes. Error: " << ec.message());
  }
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void ProxyServer::remote_read_done(const boost::system::error_code& ec, size_t cnt)
{
  if (!ec)
  {
    MYMSGS("REMOTE read done. Received: " << cnt << " bytes. Now write TUN.");

    boost::asio::const_buffer buffer(my_buf, cnt);

    proto_parser::parse_and_print_packet(my_buf);

    tun_.async_write(boost::asio::buffer(buffer),
                     boost::bind(&ProxyServer::tun_write_done, this, _1, _2));
  }
  else
  {
    MYMSGS("REMOTE read done. Received: " << cnt << " bytes. Error: " << ec.message());
  }
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void ProxyServer::tun_write_done(const boost::system::error_code& ec, size_t cnt)
{
  if (!ec)
  {
    MYMSGS("TUN write done. Wrote: " << cnt << " bytes. Now read REMOTE.");

    out_socket_.async_receive(boost::asio::buffer(my_buf, sizeof(my_buf)),
        boost::bind(&ProxyServer::remote_read_done, this, _1, _2));
  }
  else
  {
    MYMSGS("TUN write done. Wrote: " << cnt << " bytes. Error: " << ec.message());
  }
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void ProxyServer::remote_write_done(const boost::system::error_code& ec, size_t cnt)
{
  if (!ec)
  {
    MYMSGS("REMOTE write done. Wrote: " << cnt << " bytes. Now read TUN.");

    tun_.async_read(boost::asio::buffer(my_buf, sizeof(my_buf)),
                    boost::bind(&ProxyServer::tun_read_done, this, _1, _2));
  }
  else
  {
    MYMSGS("REMOTE write done. Wrote: " << cnt << " bytes. Error: " << ec.message());
  }
}
