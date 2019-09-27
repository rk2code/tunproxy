#include <gtest/gtest.h>

#include "../utils.hpp"
using namespace my_utils;

#include "../fd_handler.hpp"
using namespace my_fd;

#include "../cli_handler.hpp"
using namespace my_cli;

#include "../tun_adapter.hpp"
using namespace my_tun;


//------------------------------------------------------------------------------
TEST(utils, utils)
{
  std::string rc = sys_utils::exec("ls -la /");
  std::size_t found = rc.find("home");
  ASSERT_NE( std::string::npos, found);
}

//------------------------------------------------------------------------------
TEST(fd_handler, fd_handler)
{
  fd_handler fd(11);
  ASSERT_EQ(11, fd.native_handle());
  ASSERT_EQ( true, fd.valid());
}

//------------------------------------------------------------------------------
TEST(cli_handler, cli_handler1)
{
  std::string proxy_ip;
  int proxy_port;

  int argc = 3;
  const char *argv[3] = {"prg", "-p3000", "-i1.1.1.1"};

  int rc = cli_handler::handle_cli_options(argc, (char **) argv, proxy_ip, proxy_port);

  ASSERT_EQ(0, rc);
  ASSERT_EQ("1.1.1.1", proxy_ip);
  ASSERT_EQ(3000, proxy_port);
}
TEST(cli_handler, cli_handler2)
{
  std::string proxy_ip = "1";
  int proxy_port = 1;

  int argc = 2;
  const char *argv[2] = {"prg", "-h"};

  int rc = cli_handler::handle_cli_options(argc, (char **) argv, proxy_ip, proxy_port);

  ASSERT_EQ(-1, rc);
  ASSERT_EQ("1", proxy_ip);
  ASSERT_EQ(1, proxy_port);
}

//------------------------------------------------------------------------------
TEST(tun_adapter, tun_adapter)
{
}

//------------------------------------------------------------------------------
TEST(proto_parser, proto_parser)
{
  try
  {
    //ASSERT_EQ(0, proto_parser::proto_parser("AA", "S"));
  }
  catch (...)
  {
    SUCCEED();
  }
}

//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
