#include <boost/program_options.hpp>
#include <iostream>

#include <boost/asio.hpp>

#include "cli_handler.hpp"
using namespace my_cli;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int cli_handler::handle_cli_options(int argc, char* argv[], std::string& ip, int& port)
{
  std::string proxy_ip;
  int proxy_port = 3000;
  try
  {
    boost::program_options::options_description desc{"Options"};
    desc.add_options()
        ("help,h", "Program usage information")
        ("proxy_ip,i", boost::program_options::value<std::string>(&proxy_ip)->required(), "Proxy IP address e.g. 192.168.1.10")
        ("proxy_port,p", boost::program_options::value<int>(&proxy_port)->required(), "Proxy port e.g. 3000");

    boost::program_options::variables_map vm;
    boost::program_options::store(parse_command_line(argc, argv, desc), vm);

    if (vm.count("help"))
    {
      std::cout << desc << "\n\n";
      return -1;
    }
    else
      notify(vm);
  }
  catch (const boost::program_options::error &ex)
  {
    std::cerr << "\n" << ex.what() << "\n\n";
    return -1;
  }

  // validate IP address and port
  boost::system::error_code ec;
  boost::asio::ip::address::from_string(proxy_ip, ec);
  if (ec)
  {
    std::cerr << "\nInvalid Proxy IP address: " << proxy_ip << "\n\n";
    return -1;
  }

  ip = proxy_ip;
  port = proxy_port;
  return 0;
}
