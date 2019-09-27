#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>

#include <sstream>

#include "proto_parser.hpp"
using namespace my_proto;

#include "trace.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string proto_parser::proto_name(unsigned int pid)
{
  std::stringstream ss;
  ss << pid;
  switch (pid)
  {
    case proto_icmp: return "icmp/"+ss.str();
    case proto_tcp: return "tcp/"+ss.str();
    case proto_udp: return "udp/"+ss.str();
  }
  return "proto_unkn/"+ss.str();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void proto_parser::parse_and_print_packet(char* buf)
{
  static int tcp_cnt=0, udp_cnt=0, icmp_cnt=0, others_cnt=0, total_cnt=0;
  struct iphdr *iph = (struct iphdr*)buf;

  ++total_cnt;

  if (is_ip6(buf))
  {
    MYMSGS("IPv6 packet received - skipping...");
    return;
  }

  print_ip_header(buf);

  switch (iph->protocol) //Check the Protocol and do accordingly...
  {
    case proto_icmp:  //ICMP Protocol
      // to send ICMP traffic: ping 10.0.0.11
      ++icmp_cnt;
      print_icmp(buf);
      break;

    case proto_tcp:  //TCP Protocol
      // to send TCP traffic:
      //   echo "Hello!" | netcat 10.0.0.11 3000
      //   wget 10.0.0.11
      //   curl --socks5 127.0.0.1:1080 http://google.com/
      ++tcp_cnt;
      print_tcp(buf);
      break;

    case proto_udp: //UDP Protocol
      // to send UDP traffic: echo "This is my data" > /dev/udp/10.0.0.55/3000
      ++udp_cnt;
      print_udp(buf);
      break;

    default: //Some Other Protocol like ARP etc.
      ++others_cnt;
      break;
  }
  MYMSGS("TCP: " << tcp_cnt
                << " UDP: " << udp_cnt
                << " ICMP: " << icmp_cnt
                << " Others: " << others_cnt
                << " Total: " << total_cnt << "\r");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool proto_parser::is_udp(char* buf)
{
  struct iphdr *iph = (struct iphdr*)buf;
  return (iph != NULL && iph->protocol == 17);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool proto_parser::is_tcp(char* buf)
{
  struct iphdr *iph = (struct iphdr*)buf;
  return (iph != NULL && iph->protocol == 6);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool proto_parser::is_ip4(char* buf)
{
  struct iphdr *iph = (struct iphdr*)buf;
  return (iph != NULL && iph->version == 4);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool proto_parser::is_ip6(char* buf)
{
  struct iphdr *iph = (struct iphdr*)buf;
  return (iph != NULL && iph->version == 6);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void proto_parser::print_ip_header(char *buf)
{
  struct iphdr *iph = (struct iphdr *)buf;
  struct sockaddr_in src, dst;

  memset(&src, 0, sizeof(src));
  src.sin_addr.s_addr = iph->saddr;

  memset(&dst, 0, sizeof(dst));
  dst.sin_addr.s_addr = iph->daddr;

  MYMSGS("IP HEADER info >>>>>");
  MYMSGS("    IP version      : " << iph->version);
  MYMSGS("    Protocol        : " << proto_name((proto_id)iph->protocol));
  MYMSGS("    Packet size     : " << ntohs(iph->tot_len));
  MYMSGS("    Source IP       : " << inet_ntoa(src.sin_addr));
  MYMSGS("    Destination IP  : " << inet_ntoa(dst.sin_addr));
  MYMSGS("");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void proto_parser::print_tcp(char *buf)
{
  unsigned short iphdrlen;
  struct iphdr *iph = (struct iphdr *)buf;
  iphdrlen = iph->ihl*4;
  struct tcphdr *tcph=(struct tcphdr*)(buf + iphdrlen);

  MYMSGS("TCP HEADER info >>>>>");
  MYMSGS("    Source Port      : " << ntohs(tcph->source));
  MYMSGS("    Destination Port : " << ntohs(tcph->dest));
  MYMSGS("");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void proto_parser::print_udp(char *buf)
{
  unsigned short iphdrlen;
  struct iphdr *iph = (struct iphdr *)buf;
  iphdrlen = iph->ihl*4;
  struct udphdr *udph = (struct udphdr*)(buf + iphdrlen);

  MYMSGS("UDP HEADER info >>>>>");
  MYMSGS("    Source Port      : " << ntohs(udph->source));
  MYMSGS("    Destination Port : " << ntohs(udph->dest));
  MYMSGS("");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void proto_parser::print_icmp(char *buf)
{
  unsigned short iphdrlen;
  struct iphdr *iph = (struct iphdr *)buf;
  iphdrlen = iph->ihl*4;
  struct icmphdr *icmph = (struct icmphdr *)(buf + iphdrlen);
  std::string str = (unsigned int)(icmph->type) == 11?"(TTL Expired)":"(ICMP Echo Reply)";

  MYMSGS("ICMP HEADER info >>>>>");
  MYMSGS("    Type : " << (unsigned int)(icmph->type));
  MYMSGS("    " << str);
  MYMSGS("");
}