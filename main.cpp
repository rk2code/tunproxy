#include <unistd.h>

#include "proto_parser.hpp"
using namespace my_proto;

#include "cli_handler.hpp"
using namespace my_cli;

#include "proxy.hpp"
using namespace my_proxy;

#include "trace.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <csignal>
#include <iostream>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static volatile bool signaled = false;
static boost::function<void ()> stop_function = 0;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void signal_handler(int code)
{
  //MYLOG2();
  switch (code)
  {
    case SIGTERM:
    case SIGINT:
    case SIGABRT:
      if (!signaled && stop_function)
      {
        signaled = true;
        std::cerr << "Signal caught: stopping..." << std::endl;

        stop_function();
        stop_function = 0;
      }
      break;
    default:
      break;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static bool register_signal_handlers()
{
  //MYLOG2();
  if (signal(SIGTERM, signal_handler) == SIG_ERR)
  {
    MYMSGS("Failed to catch SIGTERM signals.");
    return false;
  }

  if (signal(SIGINT, signal_handler) == SIG_ERR)
  {
    MYMSGS("Failed to catch SIGINT signals.");
    return false;
  }

  if (signal(SIGABRT, signal_handler) == SIG_ERR)
  {
    MYMSGS("Failed to catch SIGABRT signals.");
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void close_fn(boost::asio::io_service& io_service)
{
  //MYLOG2();
  io_service.stop();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  //MYLOG2();

  if (getuid())
  {
    MYMSGS("You are not root!");
    return -1;
  }

  MYMSGS("OK, you are root.");

  std::string remote_ip;
  int remote_port = 3000;

  if (cli_handler::handle_cli_options(argc, argv, remote_ip, remote_port) != 0) return -1;
  if (!register_signal_handlers()) return -1;

  try
  {
    MYMSGS("");
    MYMSGS("Starting with parameters: remote IP: " << remote_ip << " PORT: " << remote_port);
    MYMSGS("");

    boost::asio::io_service io_service;
    proxy server(io_service, remote_ip, remote_port);

    stop_function = boost::bind(&close_fn, std::ref(io_service));

    io_service.run();
  }
  catch (const std::exception& e)
  {
    MYMSGS("In Main Error: " << e.what());
  }
  catch (...)
  {
    MYMSGS("In Main Error: unknown");
  }

  return 0;
}