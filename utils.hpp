#ifndef _UTILS_
#define _UTILS_

#include <string>

#include "trace.hpp"

namespace my_utils
{
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
class sys_utils
{
public:
  static std::string exec(const string &aCmd)
  {
    //MYLOG2();

    std::string result = "";
    try
    {
      std::array<char, 128> buffer;
      std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(aCmd.c_str(), "r"), pclose);
      if (!pipe) throw std::runtime_error("popen() failed!");
      while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
      {
        result += buffer.data();
      }
    }
    catch (const std::exception &e)
    {
      MYMSGS("exec error: " << e.what() << '\n');
    }
    catch (...)
    {
      MYMSGS("exec error: UNKNOWN\n");
    }

    return result;
  }
};

}

#endif
