TUN Proxy

TUN proxy implemented using C++ Boost.Asio (using OS: Ubuntu 18.04).

Boost.Asio is a cross-platform C++ library for network and low-level I/O programming that provides developers with a consistent asynchronous model using a modern C++ approach.

TUN proxy goal: using TUN virtual interface redirect all UDP traffic to Socks5 proxy using TCP connection.

----------------
How to build:
1. you should have Ubuntu 18.04 host (docker image prepare script is in progress, but not finished, sorry);

2. you should install these tools:

2.1. cmake 3.12 (see e.g. https://peshmerge.io/how-to-install-cmake-3-11-0-on-ubuntu-16-04/);

2.2. boost (sudo apt-get install libboost-all-dev);

2.3. gtest (see: https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/);

3. create project directory: mkdir /tmp/myproject && cd $_

4. get source code: git clone https://github.com/rk2code/tunproxy.git

5. build code: cd /tmp/myproject/tunproxy && mkdir build && cd $_ && cmake ../ && make

----------------
How to run:

Note: you will need superuser (root) to execute certain steps in the following scenario.

1. go to working dir: cd /tmp/myproject/tunproxy/build/

2. in new terminal user start simple/simulated TCP server (cmd: ./tcpserver 2222) (it will bind to 127.0.0.1:2222) to receive traffic from Socks5 proxy;

3. in new terminal user start Socks5 proxy (cmd: ./socks5proxy) (it will bind to 127.0.0.1:1080);

4. in new terminal user start TUN proxy (cmd: sudo ./tunproxy -i 127.0.0.1 -p 1080):

4.1. tunproxy will create/setup TUN interface "tun0" (10.0.0.0/24);

4.2. tunproxy will connect to Socks5 proxy (perform handshake and submit connect command), start reading "tun0" traffic, recognise UDP and redirect to Socks5 proxy, which send this traffic to above mentioned TCP server;

5. in new terminal user should generate/simulate some UDP traffic in 10.0.0.0/24 subnet e.g. echo "time now: "`date` > /dev/udp/10.0.0.15/1111 and observe TCP server output it should show the same message;

----------------
TODO:
1. capture Host all UDP traffic (routing config TBD);
2. response from remote server handling TBD;
3. unit tests are partially (minimally) implemented (using GTest framework);
4. code coverage reporting TBD;
5. more build & test automation TBD (using docker & bamboo or teamcity or gitlab);
6. release build and versioning TBD;
7. more clear "how to run" doc TBD;
8. more configuration (less hardcoded params) TBD;

