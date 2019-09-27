#ifndef PROTO_PARSER_HPP
#define PROTO_PARSER_HPP

#include <string>

namespace my_proto
{
  class proto_parser
  {
    proto_parser() = delete;
  public:
    static void parse_and_print_packet(char* buf);
    static bool is_udp(char* buf);
    static bool is_tcp(char* buf);
    static bool is_ip4(char* buf);
    static bool is_ip6(char* buf);

    enum proto_id { proto_udp = 17, proto_tcp = 6, proto_icmp = 1 };
    static std::string proto_name(unsigned int pid);

    static void print_ip_header(char *buf);
    static void print_tcp(char *buf);
    static void print_udp(char *buf);
    static void print_icmp(char *buf);
  };
}

#endif
