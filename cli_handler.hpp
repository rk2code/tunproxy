#ifndef CLI_HANDLER_HPP
#define CLI_HANDLER_HPP

namespace my_cli
{
  class cli_handler
  {
  public:
    static int handle_cli_options(int argc, char* argv[], std::string& ip, int& port);
  };
}

#endif
