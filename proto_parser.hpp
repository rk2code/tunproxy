#ifndef PROTO_PARSER_HPP
#define PROTO_PARSER_HPP

#include <string>

namespace my_proto
{
  class proto_parser
  {
  public:
    static void parse_and_print_packet(unsigned char* buf, int size, const std::string& scope);
    static bool is_udp(unsigned char* buf);
    static bool is_tcp(unsigned char* buf);
    static bool is_ip4(unsigned char* buf);
    static bool is_ip6(unsigned char* buf);
    static std::string get_udp_payload(unsigned char* buf, int size);

    enum proto_id { proto_udp = 17, proto_tcp = 6, proto_icmp = 1 };
    static std::string proto_name(unsigned int pid);

    static void print_ip_header(unsigned char *buf, int size, const std::string& scope);
    static void print_tcp(unsigned char *buf, int size, const std::string& scope);
    static void print_udp(unsigned char *buf, int size, const std::string& scope);
    static void print_icmp(unsigned char *buf, int size, const std::string& scope);
    static std::string print_packet_data(unsigned char* data, int size);

    static void print_hex_ascii_line(const unsigned char *payload, int len, int offset);
    static void print_raw_data(const unsigned char *payload, int len);

  };
}

#endif
