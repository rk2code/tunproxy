#include <linux/if_tun.h>
#include <sys/sysmacros.h>
#include <sys/socket.h>

#include "tun_adapter.hpp"
using namespace my_tun;

#include "utils.hpp"
using namespace my_utils;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void tun_adapter::open(const std::string& _name)
{
  //MYLOG2();

  // TODO/FIXME: improve error handling!

  boost::system::error_code ec = boost::system::error_code();

  const std::string dev_name = "/dev/net/tun";

  if (::access(dev_name.c_str(), F_OK) == -1)
  {
    if (errno != ENOENT)
    {
      // Unable to access the tap adapter yet it exists: this is an error.
      return;
    }

    // No tun found, create one.
    if (::mknod(dev_name.c_str(), S_IFCHR | S_IRUSR | S_IWUSR, ::makedev(10, 200)) == -1)
    {
      ec = boost::system::error_code(errno, boost::system::system_category());
      return;
    }
  }

  fd_handler device = open_device(dev_name);

  if (!device.valid())
  {
    return;
  }

  struct ifreq ifr {};

  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

  if (!_name.empty())
  {
    strncpy(ifr.ifr_name, _name.c_str(), IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0x00;
  }

  // Set the parameters on the tun device.
  if (::ioctl(device.native_handle(), TUNSETIFF, (void *)&ifr) < 0)
  {
    return;
  }

  fd_handler socket = open_socket(AF_INET);

  if (!socket.valid())
  {
    return;
  }

  m_name = std::string(ifr.ifr_name);

  if (descriptor().assign(device.release(), ec))
  {
    return;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
fd_handler tun_adapter::open_device(const std::string& name)
{
  //MYLOG2();

  const int device_fd = ::open(name.c_str(), O_RDWR);
  if (device_fd < 0) return fd_handler();
  return fd_handler(device_fd);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
fd_handler tun_adapter::open_socket(int family)
{
  //MYLOG2();

  const int socket_fd = ::socket(family, SOCK_DGRAM, 0);
  if (socket_fd < 0) return fd_handler();
  return fd_handler(socket_fd);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void tun_adapter::set_connected_state(bool connected)
{
  fd_handler socket = open_socket(AF_INET);

  struct ifreq netifr {};

  strncpy(netifr.ifr_name, name().c_str(), IFNAMSIZ);
  netifr.ifr_name[IFNAMSIZ - 1] = 0x00;

  // Get the interface flags
  if (::ioctl(socket.native_handle(), SIOCGIFFLAGS, static_cast<void*>(&netifr)) < 0)
  {
    throw boost::system::system_error(errno, boost::system::system_category());
  }

  if (connected)
  {
    netifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
  }
  else
  {
    netifr.ifr_flags &= ~(IFF_UP | IFF_RUNNING);
  }

  // Set the interface UP
  if (::ioctl(socket.native_handle(), SIOCSIFFLAGS, static_cast<void*>(&netifr)) < 0)
  {
    throw boost::system::system_error(errno, boost::system::system_category());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void tun_adapter::configure()
{
  // TODO/FIXME: make it configurable!
  std::string tunaddr = "10.0.0.15";
  std::string tunroute = "10.0.0.0/24";
  MYMSGS(sys_utils::exec("ip route add dev " + m_name + " " + tunaddr));
  MYMSGS(sys_utils::exec("ip address add dev " + m_name + " local " + tunroute));
}
