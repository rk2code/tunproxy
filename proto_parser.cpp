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
void proto_parser::parse_and_print_packet(unsigned char* buf, int size, const std::string& scope)
{
  static int tcp_cnt=0, udp_cnt=0, icmp_cnt=0, others_cnt=0, total_cnt=0;
  struct iphdr *iph = (struct iphdr*)buf;

  ++total_cnt;

  if (is_ip6(buf))
  {
    MYMSGS("IPv6 packet received - skipping...");
    return;
  }

  MYMSGS("<<<" << scope << ">>>");

  // print raw data
  print_raw_data(buf, size);

  // print structure info
  print_ip_header(buf, size, scope);

  switch (iph->protocol) //Check the Protocol and do accordingly...
  {
    case proto_icmp:  //ICMP Protocol
      // to send ICMP traffic: ping 10.0.0.11
      ++icmp_cnt;
      print_icmp(buf, size, scope);
      break;

    case proto_tcp:  //TCP Protocol
      // to send TCP traffic:
      //   echo "Hello!" | netcat 10.0.0.11 3000
      //   wget 10.0.0.11
      //   curl --socks5 127.0.0.1:1080 http://google.com/
      ++tcp_cnt;
      print_tcp(buf, size, scope);
      break;

    case proto_udp: //UDP Protocol
      // to send UDP traffic: echo "This is my data" > /dev/udp/10.0.0.55/3000
      ++udp_cnt;
      print_udp(buf, size, scope);
      break;

    default: //Some Other Protocol like ARP etc.
      ++others_cnt;
      break;
  }
  MYMSGS("Stats TCP: " << tcp_cnt
                << " UDP: " << udp_cnt
                << " ICMP: " << icmp_cnt
                << " Others: " << others_cnt
                << " Total: " << total_cnt);
  MYMSGS("");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool proto_parser::is_udp(unsigned char* buf)
{
  struct iphdr *iph = (struct iphdr*)buf;
  return (iph != NULL && iph->protocol == 17);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool proto_parser::is_tcp(unsigned char* buf)
{
  struct iphdr *iph = (struct iphdr*)buf;
  return (iph != NULL && iph->protocol == 6);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool proto_parser::is_ip4(unsigned char* buf)
{
  struct iphdr *iph = (struct iphdr*)buf;
  return (iph != NULL && iph->version == 4);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool proto_parser::is_ip6(unsigned char* buf)
{
  struct iphdr *iph = (struct iphdr*)buf;
  return (iph != NULL && iph->version == 6);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void proto_parser::print_ip_header(unsigned char *buf, int size, const std::string& scope)
{
  struct iphdr *iph = (struct iphdr *)buf;
  struct sockaddr_in src, dst;

  memset(&src, 0, sizeof(src));
  src.sin_addr.s_addr = iph->saddr;

  memset(&dst, 0, sizeof(dst));
  dst.sin_addr.s_addr = iph->daddr;

  MYMSGS("SCOPE: " << scope);
  MYMSGS("IP HEADER info >>>>>");
  MYMSGS("    IP version      : " << iph->version);
  MYMSGS("    Protocol        : " << proto_name((proto_id)iph->protocol));
  MYMSGS("    Packet size     : " << ntohs(iph->tot_len) << " (from IP header field)");
  MYMSGS("    Buffer size     : " << size << " (received size)");
  MYMSGS("    Source IP       : " << inet_ntoa(src.sin_addr));
  MYMSGS("    Destination IP  : " << inet_ntoa(dst.sin_addr));
  MYMSGS("");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void proto_parser::print_tcp(unsigned char *buf, int size, const std::string& scope)
{
  struct iphdr *iph = (struct iphdr *)buf;
  unsigned short iphdrlen = iph->ihl * 4;
  struct tcphdr *tcph = (struct tcphdr*)(buf + iphdrlen);

  unsigned short tcphdrlen = tcph->doff * 4 - sizeof(struct tcphdr);
  unsigned short payloadlen = size - iphdrlen - tcphdrlen;

  MYMSGS("1 IP HLEN[" << iphdrlen << "] TCP HLEN[" << tcphdrlen << "] PayloadLEN[" << payloadlen << "]");

  MYMSGS("SCOPE: " << scope);
  MYMSGS("TCP HEADER info >>>>>");
  MYMSGS("    Source Port      : " << ntohs(tcph->source));
  MYMSGS("    Destination Port : " << ntohs(tcph->dest));
  MYMSGS("Data Payload [sz=" << payloadlen << "]: " << proto_parser::print_packet_data(buf + iphdrlen + tcphdrlen, payloadlen));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void proto_parser::print_udp(unsigned char *buf, int size, const std::string& scope)
{
  struct iphdr *iph = (struct iphdr *)buf;
  unsigned short iphdrlen = iph->ihl * 4;
  struct udphdr *udph = (struct udphdr *) (buf + iphdrlen);
  unsigned short udphdrlen = sizeof (struct udphdr);
  unsigned short payloadlen = size - iphdrlen - udphdrlen;

  MYMSGS("SCOPE: " << scope);
  MYMSGS("UDP HEADER info >>>>>");
  MYMSGS("    Source Port      : " << ntohs(udph->source));
  MYMSGS("    Destination Port : " << ntohs(udph->dest));
  MYMSGS("Data Payload [sz=" << payloadlen << "]: " << proto_parser::print_packet_data(buf + iphdrlen + udphdrlen, payloadlen));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void proto_parser::print_icmp(unsigned char *buf, int size, const std::string& scope)
{
  unsigned short iphdrlen;
  struct iphdr *iph = (struct iphdr *)buf;
  iphdrlen = iph->ihl*4;
  struct icmphdr *icmph = (struct icmphdr *)(buf + iphdrlen);
  std::string str = (unsigned int)(icmph->type) == 11?"(TTL Expired)":"(ICMP Echo Reply)";

  MYMSGS("SCOPE: " << scope);
  MYMSGS("ICMP HEADER info >>>>>");
  MYMSGS("    Type : " << (unsigned int)(icmph->type));
  MYMSGS("    " << str);
  MYMSGS("");
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
std::string proto_parser::print_packet_data(unsigned char* data, int size)
{
  int i, j;
  std::string rc;
  std::stringstream ss;
  ss.write((const char*)data, size);

  rc = ss.str();
  return rc;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string proto_parser::get_udp_payload(unsigned char* buf, int size)
{
  struct iphdr *iph = (struct iphdr *)buf;
  unsigned short iphdrlen = iph->ihl * 4;
  struct udphdr *udph = (struct udphdr *) (buf + iphdrlen);
  unsigned short udphdrlen = sizeof (struct udphdr);
  unsigned short payloadlen = size - iphdrlen - udphdrlen;

  std::stringstream ss;
  ss.write((const char*)(buf + iphdrlen + udphdrlen), payloadlen);

  return ss.str();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void proto_parser::print_hex_ascii_line(const unsigned char *payload, int len, int offset)
{
  int i, gap;
  const unsigned char *ch;

  /* offset */
  printf("%05d   ", offset);

  /* hex */
  ch = payload;
  for(i = 0; i < len; i++) {
    printf("%02x ", *ch);
    ch++;
    /* print extra space after 8th byte for visual aid */
    if (i == 7)
      printf(" ");
  }
  /* print space to handle line less than 8 bytes */
  if (len < 8)
    printf(" ");

  /* fill hex gap with spaces if not full line */
  if (len < 16) {
    gap = 16 - len;
    for (i = 0; i < gap; i++) {
      printf("   ");
    }
  }
  printf("   ");

  /* ascii (if printable) */
  ch = payload;
  for(i = 0; i < len; i++) {
    if (isprint(*ch))
      printf("%c", *ch);
    else
      printf(".");
    ch++;
  }

  printf("\n");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void proto_parser::print_raw_data(const unsigned char *payload, int len)
{
  int len_rem = len;
  int line_width = 16;			/* number of bytes per line */
  int line_len;
  int offset = 0;					/* zero-based offset counter */
  const unsigned char *ch = payload;

  if (len <= 0)
    return;

  /* data fits on one line */
  if (len <= line_width) {
    proto_parser::print_hex_ascii_line(ch, len, offset);
    return;
  }

  /* data spans multiple lines */
  for ( ;; ) {
    /* compute current line length */
    line_len = line_width % len_rem;
    /* print line */
    proto_parser::print_hex_ascii_line(ch, line_len, offset);
    /* compute total remaining */
    len_rem = len_rem - line_len;
    /* shift pointer to remaining bytes to print */
    ch = ch + line_len;
    /* add offset */
    offset = offset + line_width;
    /* check if we have line width chars or less */
    if (len_rem <= line_width) {
      /* print last line and get out */
      proto_parser::print_hex_ascii_line(ch, len_rem, offset);
      break;
    }
  }
}
