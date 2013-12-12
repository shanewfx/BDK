#ifndef BASE_DEV_KIT_SOCKETS_H
#define BASE_DEV_KIT_SOCKETS_H

#include <Winsock2.h>
#include "stdint.h"
#include <string>
using std::string;

namespace BDK {
namespace sockets {

int      startupWinsock(); 
void     cleanupWinsock();

SOCKET   create();

int      bind(SOCKET sockfd, const struct sockaddr_in& addr);
int      listen(SOCKET sockfd);
SOCKET   accept(SOCKET sockfd, struct sockaddr_in* addr);
int      connect(SOCKET sockfd, const struct sockaddr_in& addr);

int      read(SOCKET sockfd, char* buf, int bufSize);
int      write(SOCKET sockfd, const char* buf, int dataSize);

void     close(SOCKET sockfd);

void     toHostPort(char* buf, int size, const struct sockaddr_in& addr);
void     fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

struct sockaddr_in getLocalAddr(SOCKET sockfd);
struct sockaddr_in getPeerAddr(SOCKET sockfd);

int      setNonBlock(SOCKET sockfd, bool blocking);
int      getNetError();

class InetAddress
{
public:
    InetAddress();

    explicit InetAddress(uint16_t port);

    InetAddress(const string& ip, uint16_t port);

    InetAddress(const struct sockaddr_in& addr)
        : m_addr(addr)
    { }

    string toHostPort() const;

    const struct sockaddr_in& getSockAddrInet() const { return m_addr; }
    void setSockAddrInet(const struct sockaddr_in& addr) { m_addr = addr; }

private:
    struct sockaddr_in m_addr;
};

}//namespace sockets
}

#endif//BASE_DEV_KIT_SOCKETS_H