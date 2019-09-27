#ifndef TRACE_HPP
#define TRACE_HPP

#define _DEBUG_


#ifdef _DEBUG_

#include <string>
using std::string;

#include <map>
using std::map;

#include <fstream>
using std::ofstream;

#include <iostream>
using std::cout;

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

class my_trace
{
  typedef map<string, string> StrStrMap;
  typedef map<string, int> StrIntMap;

  static int& scope()
  {
    return getCrrThreadIdentLevel();
  }
  std::string mMsg;
public:
  my_trace() { logMsg("", msgMode::modeIn); scope()++; }

  explicit my_trace(const string& aMsg) { mMsg = aMsg; logMsg(mMsg, msgMode::modeIn); scope()++; }

  ~my_trace() { scope()--; if (scope() < 0) scope()=0; logMsg(mMsg, msgMode::modeOut); }

  void use() const {}

  enum class msgMode { modeIn, modeOut, modeMsg };

  static void logMsg(const string& aMsg, msgMode aIn = msgMode::modeMsg)
  {
    string scpStr;
    int identLevel = scope();

    string smb = " ";
    switch (aIn)
    {
      case msgMode::modeIn: smb = "+"; break;
      case msgMode::modeOut: smb = "-"; break;
      case msgMode::modeMsg: smb = " "; break;
    }

    for (int i = 0; i <= identLevel; i++) scpStr += smb;

    string thId = getThreadId();

    cout << scpStr << "[" << thId << "]" << " " << aMsg << std::endl;

#ifdef _DEBUG_SEPARATE_THREAD_LOG_
        writeToFile("00", scpStr+"["+thId+"] "+aMsg);
        writeToFile(thId, scpStr+"["+thId+"] "+aMsg);
#endif
  }

  static void logMsg2File(const string& aMsg)
  {
    string thId = getThreadId();
    writeToFile("00", "["+thId+"] "+aMsg);
    writeToFile(thId, "["+thId+"] "+aMsg);
  }

  static void logMsg2File(const string& aMsg, const string& aFl)
  {
    string thId = getThreadId();
    writeToFile(aFl, "["+thId+"] "+aMsg);
  }

  static StrIntMap& getThreadIdentLevelMap()
  {
      static StrIntMap threadIdentLevelMap;
      return threadIdentLevelMap;
  }

  static int& getCrrThreadIdentLevel()
  {
      return getThreadIdentLevelMap()[getThreadId()];
  }

  static int getThreadIdentLevel(const string& aId)
  {
      return getThreadIdentLevelMap()[aId];
  }

  static void incThreadIdentLevel(const string& aId)
  {
      int rc = getThreadIdentLevelMap()[aId];
      rc++;
      getThreadIdentLevelMap()[aId] = rc;
  }

  static void decThreadIdentLevel(const string& aId)
  {
      int rc = getThreadIdentLevelMap()[aId];
      rc--;
      if (rc < 0) rc = 0;
      getThreadIdentLevelMap()[aId] = rc;
  }

  static string getThreadLogName(const string& aId)
  {
      return string("my-"+aId+".log");
  }

  static void writeToFile(const string& aThId, const string& aMsg)
  {
    try
    {
      ofstream f(getThreadLogName(aThId), std::ofstream::out | std::ofstream::app);
      if (f.good())
      {
        f << aMsg << std::endl;
      }
    }
    catch (...)
    {}
  }

  static string getThreadId()
  {
    string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
    return threadId;  // threadNumber;
  }

  static string computeMethodName(const string& function, const string& prettyFunction)
  {
    // If the input is a constructor, it gets the beginning of the class name, not of the method.
    // That's why later on we have to search for the first parenthesys
    size_t locFunName = prettyFunction.find(function);
    size_t begin = prettyFunction.rfind(" ", locFunName) + 1;
    // Adding function.length() make this faster and also allows to handle operator parenthesys!
    size_t end = prettyFunction.find("(", locFunName + function.length());
    if (prettyFunction[end + 1] == ')')
      return (prettyFunction.substr(begin, end - begin) + "()");
    else
      return (prettyFunction.substr(begin, end - begin) + "(...)");
  }

  static inline string methodName(const string& prettyFunction)
  {
    size_t colons = prettyFunction.find("::");
    size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
    size_t end = prettyFunction.rfind("(") - begin;

    return prettyFunction.substr(begin, end) + "()";
  }

  static inline string className(const string& prettyFunction)
  {
    size_t colons = prettyFunction.find("::");
    if (colons == string::npos)
      return "::";
    size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
    size_t end = colons - begin;

    return prettyFunction.substr(begin, end);
  }
};

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
#define __COMPACT_PRETTY_FUNCTION__ mylog::computeMethodName(__FUNCTION__, __PRETTY_FUNCTION__).c_str()

//--------------------------------------------------------------------------------------------------
#define __METHOD_NAME__ mylog::methodName(__PRETTY_FUNCTION__)

//--------------------------------------------------------------------------------------------------
#define __CLASS_NAME__ mylog::className(__PRETTY_FUNCTION__)


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#define COMBINE1(X, Y) X##Y  // helper macro
#define COMBINE(X, Y) COMBINE1(X, Y)

#define MYLOG() const mylog &(COMBINE(tmp, __LINE__)) = mylog(); (COMBINE(tmp,__LINE__)).use(); // NOLINT
#define MYLOG1() const mylog &(COMBINE(tmp, __LINE__)) = mylog(__COMPACT_PRETTY_FUNCTION__); (COMBINE(tmp,__LINE__)).use(); // NOLINT
#define MYLOG2() const my_trace &(COMBINE(tmp, __LINE__)) = my_trace(__FUNCTION__); (COMBINE(tmp,__LINE__)).use(); // NOLINT
#define MYLOG3() const mylog &(COMBINE(tmp, __LINE__)) = mylog("CLASS["+__CLASS_NAME__+"] METHOD["+__METHOD_NAME__+"]"); (COMBINE(tmp,__LINE__)).use(); // NOLINT
#define MYLOG_MSG(msg) mylog::logMsg(msg);  // NOLINT
#define MYMSG1(msg) mylog::logMsg(msg);  // NOLINT
#define MYMSG(msg) mylog::logMsg(std::string(__COMPACT_PRETTY_FUNCTION__)+">>> "+msg);  // NOLINT

// stream macros
#define MYMSGS(args) do{std::stringstream ss;ss<<args;my_trace::logMsg(ss.str());}while(0)  // NOLINT
// stream macro, log to file in crr dir
#define MYMSGSF(args) do{std::stringstream ss;ss<<args;mylog::logMsg2File(ss.str());}while(0)  // NOLINT
#define MYMSGSF1(fl, args) do{std::stringstream ss;ss<<args;mylog::logMsg2File(ss.str(), fl);}while(0)  // NOLINT

#else

// Production/release mode - all defines are empty!

#define MYLOG()
#define MYLOG1()
#define MYLOG2()
#define MYLOG3()
#define MYLOG_MSG(msg)
#define MYMSG(msg)
#define MYMSG1(msg)
#define MYMSGS(args)
#define MYMSGSF(args)
#define MYMSGSF1(fl, args)


#endif


#endif  // TRACE_HPP
