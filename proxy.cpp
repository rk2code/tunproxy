#include <cstdlib>
#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include <fstream>

#include <chrono>
#include <thread>

#include <boost/asio.hpp>
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

#include "proxy.hpp"
using namespace my_proxy;

#include "proto_parser.hpp"
using namespace my_proto;

#include "trace.hpp"

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
proxy::proxy(boost::asio::io_service& io_service,
             const std::string& fwd_host,
             unsigned short fwd_port)
: tun_(io_service)
, out_socket_(io_service)
, remote_host_(fwd_host)
, remote_port_(fwd_port)
{
  //MYLOG2();

  tun_.open();
  tun_.set_connected_state(true);
  tun_.configure();

  MYMSGS("TUN interface is prepared/ready.");

  do_remote_connect();
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
proxy::~proxy()
{
  tun_.set_connected_state(false);
  tun_.cancel();
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void proxy::do_remote_connect()
{
  MYMSGS("Try to connect to remote host: " << remote_host_ << ":" << remote_port_);
  boost::asio::ip::tcp::endpoint
      endpoint(boost::asio::ip::address::from_string(remote_host_), remote_port_);

  out_socket_.async_connect(endpoint, boost::bind(&proxy::remote_connect_done, this, _1));
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void proxy::remote_connect_done(const boost::system::error_code& ec)
{
  if (!ec)
  {
    MYMSGS("Connected to " << remote_host_ << ":" << remote_port_);

    s5_auth();
  }
  else
  {
    MYMSGS("Failed to connect " << remote_host_ << ":" << remote_port_
                                << " Error: " << ec.message() << " ec: " << ec);
    MYMSGS("Will retry in 2s...");
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2s);
    do_remote_connect();
  }
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void proxy::s5_auth()
{
  try
  {
    memset(remote_data_, 0, max_data_length);
    remote_data_[0] = 0x05;
    remote_data_[1] = 0x01; // one method
    remote_data_[2] = 0x00; // no authentication

    // do Socks5 authenticate/handshake
    boost::asio::async_write( out_socket_, boost::asio::buffer(remote_data_, 3),
                              boost::bind(&proxy::s5_auth_done, this, _1, _2));
  }
  catch (const std::exception& err)
  {
    MYMSGS("S5 auth send error: " << err.what());
    // TODO/FIXME: error handling should be improved!
    throw;
  }

}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void proxy::s5_auth_done(const boost::system::error_code& ec, size_t length)
{
  try
  {
    if (ec) throw std::runtime_error("S5 auth send failed: " + ec.message());

    MYMSGS("S5 auth sent ok. Sent size: " << length);

    out_socket_.async_receive(boost::asio::buffer(remote_data_),
                              [&](boost::system::error_code ec, std::size_t length) {

                                if (ec) throw std::runtime_error("S5 auth resp recv failed: " + ec.message());

                                MYMSGS("S5 auth resp received ok. Received size: " << length);

                                s5_connect();
                              });
  }
  catch (const std::exception& err)
  {
    MYMSGS("S5 auth send error: " << err.what());
    // TODO/FIXME: error handling should be improved!
    throw;
  }
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void proxy::s5_connect()
{
  try
  {
    memset(remote_data_, 0, max_data_length);
    remote_data_[0] = 0x05;
    remote_data_[1] = 0x01; // cmd: CONNECT
    remote_data_[2] = 0x00;
    remote_data_[3] = 0x01; // addr type IPv4

    uint32_t dst_addr = inet_addr("127.0.0.1");
    std::memcpy(&remote_data_[4], &dst_addr, sizeof(uint32_t));
    uint16_t dst_port = htons(2222);
    std::memcpy(&remote_data_[8], &dst_port, sizeof(uint16_t));

    boost::asio::async_write( out_socket_, boost::asio::buffer(remote_data_, 10),
                              [&] (boost::system::error_code ec, std::size_t length) {

                                if (ec) throw std::runtime_error("S5 req send failed: " + ec.message());

                                MYMSGS("S5 req sent ok. Sent size: " << length);

                                do_tun_read();
                              });
  }
  catch (const std::exception& err)
  {
    MYMSGS("S5 connect req send error: " << err.what());
    // TODO/FIXME: error handling should be improved!
    throw;
  }

}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void proxy::do_tun_read()
{
  tun_.async_read(boost::asio::buffer(tun_data_, sizeof(tun_data_)),
                  boost::bind(&proxy::tun_read_done, this, _1, _2));
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void proxy::do_remote_read()
{
    out_socket_.async_read_some(boost::asio::buffer(remote_data_, sizeof(remote_data_)),
                                boost::bind(&proxy::remote_read_done, this, _1, _2));
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void proxy::tun_read_done(const boost::system::error_code& ec, size_t tun_data_size)
{
  if (!ec)
  {
    MYMSGS("TUN read done. Received: " << tun_data_size << " bytes.");

    proto_parser::parse_and_print_packet(tun_data_, tun_data_size, "from TUN");

    if (proto_parser::is_ip4(tun_data_) && proto_parser::is_udp(tun_data_))
    {
      std::string tmp = proto_parser::get_udp_payload(tun_data_, tun_data_size);
      boost::asio::const_buffer buffer(tmp.c_str(), tmp.size());

      boost::asio::async_write(out_socket_, boost::asio::buffer(buffer),
                               boost::bind(&proxy::remote_write_done, this, _1, _2));
    }
    else
    {
      //MYMSGS("Now read TUN.");
      do_tun_read();
    }
  }
  else
  {
    MYMSGS("TUN read done. Received: " << tun_data_size << " bytes. Error: " << ec.message() << " ec: " << ec);
  }
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void proxy::remote_read_done(const boost::system::error_code& ec, size_t remote_data_size)
{
  if (!ec)
  {
    //MYMSGS("REMOTE read done. Received: " << remote_data_size << " bytes. Now write TUN.");

    boost::asio::const_buffer buffer(remote_data_, remote_data_size);

    proto_parser::parse_and_print_packet(remote_data_, remote_data_size, "from REMOTE");

    tun_.async_write(boost::asio::buffer(buffer),
                     boost::bind(&proxy::tun_write_done, this, _1, _2));
  }
  else
  {
    MYMSGS("REMOTE read done. Received: " << remote_data_size << " bytes. Error: " << ec.message() << " ec: " << ec);
  }
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void proxy::tun_write_done(const boost::system::error_code& ec, size_t cnt)
{
  if (!ec)
  {
    //MYMSGS("TUN write done. Wrote: " << cnt << " bytes. Now read REMOTE.");
    do_remote_read();
  }
  else
  {
    MYMSGS("TUN write done. Wrote: " << cnt << " bytes. Error: " << ec.message() << " ec: " << ec);
  }
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void proxy::remote_write_done(const boost::system::error_code& ec, size_t cnt)
{
  if (!ec)
  {
    //MYMSGS("REMOTE write done. Wrote: " << cnt << " bytes. Now read TUN.");
    do_tun_read();
  }
  else
  {
    MYMSGS("REMOTE write done. Wrote: " << cnt << " bytes. Error: " << ec.message() << " ec: " << ec);
  }
}
