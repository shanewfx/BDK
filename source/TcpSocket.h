#ifndef BASE_DEV_KIT_TCP_SOCKET_H
#define BASE_DEV_KIT_TCP_SOCKET_H

#include "sockets.h"
#include <boost\noncopyable.hpp>

namespace BDK {

typedef void (*RECV_CALLBACK)(char* buf, int dataSize, void* usrData);
typedef void (*NOTIFY_CALLBACK)(uint32_t eventCode, void* param, void* usrData);

typedef struct TcpSocketCallback {
    RECV_CALLBACK    fn_recv;
    NOTIFY_CALLBACK  fn_notify;
    void*            usrData;
} TcpSocketCallback_t;

enum {
    E_NET_PEER_CLOSED,
    E_NET_CONNECT_SUCCESS,
    E_NET_CONNECT_FAILED,
    E_EVT_CNT
};

const char* getEventDescriptor(uint32_t eventCode);

class TcpServer : boost::noncopyable
{
public:
    TcpServer();
   ~TcpServer();

    void   loop();

    bool   start(const sockets::InetAddress& localAddr);
    void   stop();

    int    send(const char* buf, int size);
    void   setCallback(TcpSocketCallback_t& callback);

    bool   isRunning() const { return m_started; }

private:
    bool   bind();
    bool   listen();
    bool   accept();

    string getloaclAddrInfo();
    string getPeerAddrInfo();

    static DWORD WINAPI socketThread(LPVOID param);
    int    socketProc();
    void   wakeup();
    void   cleanup();

    bool   m_started;

    bool   m_exit;
    HANDLE m_thread;
    HANDLE m_loopEvent;

    SOCKET m_listenfd;
    SOCKET m_connfd;

    sockets::InetAddress m_localAddr;
    sockets::InetAddress m_peerAddr;

    TcpSocketCallback_t  m_callback;
};


class TcpClient : boost::noncopyable
{
public:
    TcpClient();
   ~TcpClient();

    bool   connect(const sockets::InetAddress& serverAddr);
    void   disconnect();

    int    send(const char* buf, int size);
    void   setCallback(TcpSocketCallback_t& callback);

    bool   isConnected() const { return m_connected; }

private:
    static DWORD WINAPI socketThread(LPVOID param);
    int    socketProc();
    void   wakeup();
    void   cleanup();

    string getServerAddrInfo();

    bool   m_connected;

    bool   m_exit;
    HANDLE m_thread;

    SOCKET m_connfd;
    sockets::InetAddress m_serverAddr;

    TcpSocketCallback_t  m_callback;
};

}

#endif//BASE_DEV_KIT_TCP_SOCKET_H