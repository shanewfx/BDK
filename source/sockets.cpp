#include "sockets.h"
#include <WS2tcpip.h>
#include <strsafe.h>

#pragma comment(lib, "Ws2_32.lib")

namespace BDK {
namespace sockets {

typedef struct sockaddr SA;

const SA* sockaddr_cast(const struct sockaddr_in* addr)
{
    return static_cast<const SA*>((const void*)addr);
}

SA* sockaddr_cast(struct sockaddr_in* addr)
{
    return static_cast<SA*>((void*)addr);
}

int startupWinsock()
{
    WSADATA wsaData;	
    WORD version = MAKEWORD(2, 2);
    if (::WSAStartup(version, &wsaData) != 0) {
        return -1;
    }
    return 0;
}

void cleanupWinsock()
{
    ::WSACleanup();
}

SOCKET create()
{
    SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP/*0*/);
    if (INVALID_SOCKET == sockfd) {
        return INVALID_SOCKET;
    }

    return sockfd;
}

int bind(SOCKET sockfd, const struct sockaddr_in& addr)
{
    int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof(addr));
    if (SOCKET_ERROR == ret) {
        return SOCKET_ERROR;
    }
    return ret;
}

int listen(SOCKET sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (SOCKET_ERROR == ret) {
        return SOCKET_ERROR;
    }
    return ret;
}

SOCKET accept(SOCKET sockfd, struct sockaddr_in* addr)
{
    int addrLen = sizeof(*addr);
    SOCKET connfd = ::accept(sockfd, sockaddr_cast(addr), &addrLen);
    if (INVALID_SOCKET == connfd) {
        return INVALID_SOCKET;
    }
    return connfd;
}

int connect(SOCKET sockfd, const struct sockaddr_in& addr)
{
    int ret = ::connect(sockfd, sockaddr_cast(&addr), sizeof(addr));
    if (SOCKET_ERROR == ret) {
        return SOCKET_ERROR;
    }
    return ret;
}

int read(SOCKET sockfd, char* buf, int bufSize)
{
    int ret = ::recv(sockfd, buf, bufSize, 0);
    return ret;
}

int write(SOCKET sockfd, const char* buf, int dataSize)
{
    int ret = ::send(sockfd, buf, dataSize, 0);
    return ret;
}

void close(SOCKET sockfd)
{
    ::closesocket(sockfd);
}


void toHostPort(char* buf, int size, const struct sockaddr_in& addr)
{
    char host[INET_ADDRSTRLEN] = "INVALID";
#if WINVER >= 0x0600
    ::inet_ntop(AF_INET, (PVOID)&addr.sin_addr, host, sizeof(host)); //supported on Windows Vista and later, ip4/ip6
#else
    char* ip = ::inet_ntoa(addr.sin_addr);
    strcpy_s(host, INET_ADDRSTRLEN, ip);
#endif
    uint16_t port = ::ntohs(addr.sin_port);
    _snprintf_s(buf, size, size, "%s:%u", host, port);
}

void fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port   = ::htons(port);
#if WINVER >= 0x0600
    ::inet_pton(AF_INET, ip, &addr->sin_addr); //supported on Windows Vista and later, ip4/ip6
#else
    addr->sin_addr.s_addr = ::inet_addr(ip);
#endif
}

struct sockaddr_in getLocalAddr(SOCKET sockfd)
{
    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    socklen_t addrLen = sizeof(localAddr);
    ::getsockname(sockfd, sockaddr_cast(&localAddr), &addrLen);
    return localAddr;
}

struct sockaddr_in getPeerAddr(SOCKET sockfd)
{
    struct sockaddr_in peerAddr;
    memset(&peerAddr, 0, sizeof(peerAddr));
    socklen_t addrLen = sizeof(peerAddr);
    ::getpeername(sockfd, sockaddr_cast(&peerAddr), &addrLen);
    return peerAddr;
}

int getNetError()
{
    return WSAGetLastError();
}



InetAddress::InetAddress()
{
    memset(&m_addr, 0, sizeof(m_addr));
}

InetAddress::InetAddress(uint16_t port)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    m_addr.sin_port = ::htons(port);
}

InetAddress::InetAddress(const string& ip, uint16_t port)
{
    memset(&m_addr, 0, sizeof(m_addr));
    sockets::fromHostPort(ip.c_str(), port, &m_addr);
}

string InetAddress::toHostPort() const
{
    char buf[32];
    sockets::toHostPort(buf, sizeof(buf), m_addr);
    return buf;
}

int setNonBlock(SOCKET sockfd, bool blocking)
{
    unsigned long optBlock = blocking ? 0 : 1;
    int ret = ioctlsocket(sockfd, FIONBIO, &optBlock);
    if (SOCKET_ERROR == ret) {
        return SOCKET_ERROR;
    }
    return ret;
}

}//namespace sockets
}//namespace BDK