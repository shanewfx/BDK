#include "TcpSocket.h"
#include "log.h"
#include <assert.h>

namespace BDK {

#define NOTIFY(ev, paramptr) do { if (m_callback.fn_notify) m_callback.fn_notify(ev, paramptr, m_callback.usrData); } while (0);

const char* getEventDescriptor(uint32_t eventCode)
{
    static const char* strEvent[] = {
        "E_NET_PEER_CLOSED",
        "E_NET_CONNECT_SUCCESS",
        "E_NET_CONNECT_FAILED"
    };

    if (eventCode >= E_EVT_CNT) {
        return "invalid event code";
    }
    return strEvent[eventCode];
}

TcpServer::TcpServer()
    : m_listenfd(INVALID_SOCKET)
    , m_connfd(INVALID_SOCKET)
    , m_thread(NULL)
    , m_loopEvent(NULL)
    , m_exit(false)
    , m_started(false)
#ifdef SUPPORT_MULTICLIENTS
    , m_clientCount(0)
#endif
{
    memset(&m_callback, 0, sizeof(m_callback));
#ifdef SUPPORT_MULTICLIENTS
    memset(m_clientfds, INVALID_SOCKET, sizeof(m_clientfds));
#endif
    sockets::startupWinsock();
}

TcpServer::~TcpServer()
{
    stop();
    sockets::cleanupWinsock();
}

void TcpServer::loop()
{
    LogTrace("*** server loop begin ***\n");
    WaitForSingleObject(m_loopEvent, INFINITE);
    LogTrace("*** server loop end ***\n");
}

bool TcpServer::start(const sockets::InetAddress& localAddr)
{
    if (m_started) {
        LogWarning("=== server started ===\n");
        return false;
    }
    m_localAddr = localAddr;

    if (!bind()) {
        LogError("!! -- bind failed --\n");
        goto START_FAILED;
    } 

    if (!(listen())) {
        LogError("!! -- listen failed --\n");
        goto START_FAILED;
    }

    m_loopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == m_loopEvent) {
        LogError("!! -- CreateEvent failed --\n");
        goto START_FAILED;
    }

    m_exit = false;
    m_thread = CreateThread(NULL, 0, socketThread, this, 0, NULL);
    if (NULL == m_thread) {
        LogError("!! -- CreateThread failed --\n");
        CloseHandle(m_loopEvent);
        m_loopEvent = NULL;
        goto START_FAILED;
    }

    m_started = true;
    LogTrace("=== start server {%s} success ===\n", getloaclAddrInfo().c_str());
    return true;

START_FAILED:
    cleanup();
    return false;
}

void TcpServer::stop()
{
    if (m_thread != NULL) {		
        m_exit = true;
        wakeup(); //wakeup select in order to exit thread
        LogTrace("=== wait socket thread exit begin ===\n");
        WaitForSingleObject(m_thread, INFINITE);
        LogTrace("=== wait socket thread exit end ===\n");
        CloseHandle(m_thread);
        m_thread = NULL;

        CloseHandle(m_loopEvent);
        m_loopEvent = NULL;
    }

    cleanup();

    m_started = false;
    LogTrace("=== stop server success ===\n");
}

bool TcpServer::bind()
{
    m_listenfd = sockets::create();
    if (INVALID_SOCKET == m_listenfd) {
        return false;
    }
    sockets::setNonBlock(m_listenfd, false);
    

    int ret = sockets::bind(m_listenfd, m_localAddr.getSockAddrInet());
    if (SOCKET_ERROR == ret) {
        return false;
    }
    return true;
}

bool TcpServer::listen()
{
    int ret = sockets::listen(m_listenfd);
    if (SOCKET_ERROR == ret) {
        return false;
    }
    return true;
}

bool TcpServer::accept()
{
    struct sockaddr_in addr;
    m_connfd = sockets::accept(m_listenfd, &addr);
    if (INVALID_SOCKET == m_connfd) {
        return false;
    }
    m_peerAddr.setSockAddrInet(addr);
    return true;
}

int TcpServer::send(SOCKET connfd, const char* buf, int size)
{
    if (INVALID_SOCKET == connfd) {
        return -1;
    }

    int ret = sockets::write(connfd, buf, size);
    return ret;
}

void TcpServer::setCallback(TcpSocketCallback_t& callback)
{
    m_callback = callback;
}

DWORD WINAPI TcpServer::socketThread(LPVOID param)
{
    TcpServer* tcpServer = (TcpServer*)param;
    if (NULL == tcpServer) {
        return 1;
    }

    tcpServer->socketProc();
    return 0;
}

#ifndef SUPPORT_MULTICLIENTS
int TcpServer::socketProc()
{
    fd_set fdMasterSet;
    fd_set fdWorkingSet;

    FD_ZERO(&fdMasterSet);
    FD_SET(m_listenfd, &fdMasterSet);
    SOCKET maxfd = m_listenfd;

//#define ENABLE_SELECT_TIMEOUT
#ifdef ENABLE_SELECT_TIMEOUT
    timeval timeout;
    timeout.tv_sec  = 3 * 60;
    timeout.tv_usec = 0;
#endif

    while (!m_exit) {
        memcpy(&fdWorkingSet, &fdMasterSet, sizeof(fdMasterSet));

        int ret = select(maxfd + 1/*0*/, &fdWorkingSet, NULL, NULL, 
#ifdef ENABLE_SELECT_TIMEOUT
            &timeout
#else
            NULL
#endif
            );
        if (ret < 0) {
            LogError("@@@ select error, exit thread @@@\n");
            break;
        }
#ifdef ENABLE_SELECT_TIMEOUT
        else if (0 == ret) {
            //timeout
            continue;
        }
#endif
        else {
            if (m_exit) {
                LogTrace("@@@ socket thread wakeup, exit thread @@@\n");
                break; //exit thread
            }

            //fd active
#if 0
            for (SOCKET s = 0; s <= maxfd; s++) {
                if (FD_ISSET(s, &fdWorkingSet)) {
                    if (s == m_listenfd) {
                        if (m_connfd != INVALID_SOCKET) {
                            LogTrace("== new connect income, close old connect socket ==\n");
                            sockets::close(m_connfd);
                            m_connfd = INVALID_SOCKET;
                        }

                        if (accept()) {
                            FD_SET(m_connfd, &fdMasterSet);
                            if (m_connfd > maxfd) {
                                maxfd = m_connfd;
                            }
                        }
                    }
                    else {
                        //connfd, to recv data
                        assert(m_connfd != INVALID_SOCKET);

                        enum {
                            RECV_BUF_SIZE = 512 * 1024
                        };
                        char buf[RECV_BUF_SIZE];
                        memset(buf, 0, sizeof(buf));

                        int dataLen = sockets::read(m_connfd, buf, RECV_BUF_SIZE);
                        if ((dataLen == 0 || dataLen == SOCKET_ERROR) && m_connfd != INVALID_SOCKET) {
                            FD_CLR(m_connfd, &fdMasterSet);
                            maxfd = m_listenfd;

                            if (dataLen == SOCKET_ERROR) {
                                LogWarning("!! -- recv failed, err: %d --\n", sockets::getNetError());
                            }
                            else {
                                LogWarning("@@ -- recv failed, client closed --\n");
                            }
                            LogWarning("===== close client socket (m_connfd: %d) =====\n", m_connfd);

                            sockets::close(m_connfd);
                            m_connfd = INVALID_SOCKET;
                        }

                        if (dataLen > 0 && m_callback.fn_recv) {
                            m_callback.fn_recv(m_connfd, buf, dataLen, m_callback.usrData);
                        }
                    }
                }
            }
#else
            for (int i = 0; i < fdWorkingSet.fd_count; i++) {
                SOCKET s = fdWorkingSet.fd_array[i];
                if (!FD_ISSET(s, &fdWorkingSet)) {
                    continue;
                }

                if (s == m_listenfd) {
                    if (m_connfd != INVALID_SOCKET) {
                        LogTrace("== new connect request, close old client socket (m_connfd: %d) ==\n", m_connfd);
                        sockets::close(m_connfd);
                        m_connfd = INVALID_SOCKET;
                    }

                    if (accept()) {
                        LogTrace("== accpet new connect request, { %s }, m_connfd: %d ==\n", getPeerAddrInfo().c_str(), m_connfd);
                        FD_SET(m_connfd, &fdMasterSet);
                        if (m_connfd > maxfd) {
                            maxfd = m_connfd;
                        }
                    }
                }
                else {
                    //connfd, to recv data
                    assert(m_connfd != INVALID_SOCKET);

                    enum {
                        RECV_BUF_SIZE = 512 * 1024
                    };
                    char buf[RECV_BUF_SIZE];
                    memset(buf, 0, sizeof(buf));

                    int dataLen = sockets::read(m_connfd, buf, RECV_BUF_SIZE);
                    if ((dataLen == 0 || dataLen == SOCKET_ERROR) && m_connfd != INVALID_SOCKET) {
                        FD_CLR(m_connfd, &fdMasterSet);
                        maxfd = m_listenfd;

                        if (dataLen == SOCKET_ERROR) {
                            LogWarning("!! -- recv failed, err: %d --\n", sockets::getNetError());
                        }
                        else {
                            LogWarning("@@ -- recv failed, client closed --\n");
                        }
                        LogWarning("===== close client socket (m_connfd: %d) =====\n", m_connfd);

                        sockets::close(m_connfd);
                        m_connfd = INVALID_SOCKET;    
                        continue;
                    }

                    if (dataLen > 0 && m_callback.fn_recv) {
                        m_callback.fn_recv(m_connfd, buf, dataLen, m_callback.usrData);
                    }
                }
            }
#endif
        }
    }

    SetEvent(m_loopEvent); //signal event to exit loop
    return 0;
}

#else

int TcpServer::socketProc()
{
    fd_set fdWorkingSet;
    SOCKET maxfd = m_listenfd;

    //#define ENABLE_SELECT_TIMEOUT
#ifdef ENABLE_SELECT_TIMEOUT
    timeval timeout;
    timeout.tv_sec  = 3 * 60;
    timeout.tv_usec = 0;
#endif

    while (!m_exit) {
        FD_ZERO(&fdWorkingSet);
        FD_SET(m_listenfd, &fdWorkingSet);
        maxfd = m_listenfd;

        for (int i = 0; i < MAXCLIENTCOUNT; i++) {
            if (m_clientfds[i] != INVALID_SOCKET) {
                //LogTrace("add to set, i: %d, socket: %d\n", i, m_clientfds[i]);
                FD_SET(m_clientfds[i], &fdWorkingSet);
                if (m_clientfds[i] > maxfd) {
                    maxfd = m_clientfds[i];
                }
            }
        }

        int ret = select(maxfd + 1/*0*/, &fdWorkingSet, NULL, NULL, 
#ifdef ENABLE_SELECT_TIMEOUT
            &timeout
#else
            NULL
#endif
            );
        if (ret < 0) {
            LogError("@@@ select error, exit thread @@@\n");
            break;
        }
#ifdef ENABLE_SELECT_TIMEOUT
        else if (0 == ret) {
            //timeout
            continue;
        }
#endif
        else {
            if (m_exit) {
                LogTrace("@@@ socket thread wakeup, exit thread @@@\n");
                break; //exit thread
            }

            //fd active
            for (int i = 0; i < fdWorkingSet.fd_count; i++) {
                SOCKET s = fdWorkingSet.fd_array[i];
                if (!FD_ISSET(s, &fdWorkingSet)) {
                    continue;
                }
                //LogTrace("fd active: %d, fd_count: %d, idx: %d, socket: %d\n", ret, fdWorkingSet.fd_count, i, s);

                if (s == m_listenfd) {
                    if (accept()) {
                        if (m_clientCount >= MAXCLIENTCOUNT) {
                            sockets::close(m_connfd);
                            m_connfd = INVALID_SOCKET; 
                            LogWarning("!! -- reject connect, m_clientCount (%d) >=  MAXCLIENTCOUNT(%d) --\n", m_clientCount, MAXCLIENTCOUNT);
                            continue;
                        }

                        for (int j = 0; j < MAXCLIENTCOUNT; j++) {
                            if (m_clientfds[j] == INVALID_SOCKET) {
                                m_clientfds[j] = m_connfd;
                                m_clientCount++;
                                break;
                            }
                        }

                        LogTrace("== accpet new connect request, { %s }, m_connfd: %d, m_clientCount: %d ==\n", getPeerAddrInfo().c_str(), m_connfd, m_clientCount);       
                    }
                }
                else {
                    //connfd, to recv data
                    int j = 0;
                    for (; j < MAXCLIENTCOUNT; j++) {
                        if (m_clientfds[j] == s) {
                            break;
                        }
                    }

                    if (j >= MAXCLIENTCOUNT) {
                        continue;
                    }
                    SOCKET& connfd = m_clientfds[j];


                    enum {
                        RECV_BUF_SIZE = 512 * 1024
                    };
                    char buf[RECV_BUF_SIZE];
                    memset(buf, 0, sizeof(buf));

                    int dataLen = sockets::read(connfd, buf, RECV_BUF_SIZE);
                    if ((dataLen == 0 || dataLen == SOCKET_ERROR) && connfd != INVALID_SOCKET) {
                        if (dataLen == SOCKET_ERROR) {
                            LogWarning("!! -- recv failed, err: %d --\n", sockets::getNetError());
                        }
                        else {
                            LogWarning("@@ -- recv failed, client closed --\n");
                        }
                        LogWarning("===== close client socket (connfd: %d) =====\n", connfd);

                        sockets::close(connfd);
                        connfd = INVALID_SOCKET;  
                        m_clientCount--;
                        continue;
                    }

                    if (dataLen > 0 && m_callback.fn_recv) {
                        m_callback.fn_recv(connfd, buf, dataLen, m_callback.usrData);
                    }
                }
            }
        }
    }

    for (int i = 0; i < MAXCLIENTCOUNT; i++) {
        if (m_clientfds[i] != INVALID_SOCKET) {
            sockets::close(m_clientfds[i]);
            m_clientfds[i] = INVALID_SOCKET; 
        }
    }

    SetEvent(m_loopEvent); //signal event to exit loop
    return 0;
}
#endif

void TcpServer::wakeup()
{
    if (m_listenfd != INVALID_SOCKET) {
        sockets::close(m_listenfd);
        m_listenfd = INVALID_SOCKET;
    }
}

void TcpServer::cleanup()
{
    if (m_listenfd != INVALID_SOCKET) {
        sockets::close(m_listenfd);
        m_listenfd = INVALID_SOCKET;
    }

#ifdef SUPPORT_MULTICLIENTS
    for (int i = 0; i < MAXCLIENTCOUNT; i++) {
        if (m_clientfds[i] != INVALID_SOCKET) {
            sockets::close(m_clientfds[i]);
            m_clientfds[i] = INVALID_SOCKET; 
            break;
        }
    }
    m_connfd = INVALID_SOCKET;

#else

    if (m_connfd != INVALID_SOCKET) {
        sockets::close(m_connfd);
        m_connfd = INVALID_SOCKET;
    }
#endif
}

string TcpServer::getPeerAddrInfo()
{
    char buf[32] = {0};
    sockets::toHostPort(buf, 32, m_peerAddr.getSockAddrInet());
    return string(buf);
}

string TcpServer::getloaclAddrInfo()
{
    char buf[32] = {0};
    sockets::toHostPort(buf, 32, m_localAddr.getSockAddrInet());
    return string(buf);
}

/**************************************************/

TcpClient::TcpClient()
    : m_connected(false)
    , m_connfd(INVALID_SOCKET)
    , m_exit(false)
    , m_thread(NULL)
{
    memset(&m_callback, 0, sizeof(m_callback));
    sockets::startupWinsock();
}

TcpClient::~TcpClient()
{
    disconnect();
    sockets::cleanupWinsock();
}

bool TcpClient::connect(const sockets::InetAddress& serverAddr)
{
    if (m_connected) {
        return false;
    }

    if (m_connfd != INVALID_SOCKET) {
        disconnect();
        LogWarning("[connect] maybe reconnect server, disconnect called\n");
    }

    m_connfd = sockets::create();
    if (INVALID_SOCKET == m_connfd) {
        return false;
    }
    sockets::setNonBlock(m_connfd, false);
    LogTrace("client socket: %d\n", m_connfd);

    m_serverAddr = serverAddr;
    m_connected  = false;

    m_exit = false;
    m_thread = CreateThread(NULL, 0, socketThread, this, 0, NULL);
    if (NULL == m_thread) {
        LogError("!! -- CreateThread failed --\n");
        goto CONNECT_FAILED;
    }

    return true;

CONNECT_FAILED:
    cleanup();
    return false;
}

void TcpClient::disconnect()
{
    if (m_thread != NULL) {		
        m_exit = true;
        wakeup(); //wakeup select in order to exit thread
        LogTrace("=== wait socket thread exit begin ===\n");
        WaitForSingleObject(m_thread, INFINITE);
        LogTrace("=== wait socket thread exit end ===\n");
        CloseHandle(m_thread);
        m_thread = NULL;
    }

    cleanup();

    m_connected = false;
    LogTrace("=== client disconnect success ===\n");
}

int TcpClient::send(const char* buf, int size)
{
    if (INVALID_SOCKET == m_connfd) {
        return -1;
    }

    if (!m_connected) {
        return -2;
    }

    int ret = sockets::write(m_connfd, buf, size);
    return ret;
}

void TcpClient::setCallback(TcpSocketCallback_t& callback)
{
    m_callback = callback;
}

DWORD WINAPI TcpClient::socketThread(LPVOID param)
{
    TcpClient* tcpClient = (TcpClient*)param;
    if (NULL == tcpClient) {
        return 1;
    }
    tcpClient->socketProc();
    return 0;
}

int TcpClient::socketProc()
{
    int ret = sockets::connect(m_connfd, m_serverAddr.getSockAddrInet());
    if (ret == SOCKET_ERROR && sockets::getNetError() != WSAEWOULDBLOCK) {
        int err = sockets::getNetError();
        LogError("!! -- connect failed, err: %d, exit thread --\n", err);
        NOTIFY(E_NET_CONNECT_FAILED, &err);
        return 1;
    }
    

    fd_set fdRead;
    fd_set fdWrite;
    FD_ZERO(&fdRead);
    FD_ZERO(&fdWrite);
    FD_SET(m_connfd, &fdRead);
    FD_SET(m_connfd, &fdWrite);

    timeval timeout;
    timeout.tv_sec  = 5;
    timeout.tv_usec = 0;

    while (!m_exit) {
        if (m_connected) {
            FD_SET(m_connfd, &fdRead);
        }
        else {
            FD_SET(m_connfd, &fdRead);
            FD_SET(m_connfd, &fdWrite);
        }

        ret = select(m_connfd + 1/*0*/, &fdRead, &fdWrite, NULL, &timeout); 
        if (ret < 0) {
            LogError("@@@ select error, exit thread @@@\n");
            break;
        }
        else if (ret == 0) {
            if (!m_connected) {
                LogError("!! -- select timeout, connect failed, exit thread --\n");
                NOTIFY(E_NET_CONNECT_FAILED, NULL);
                break;
            }
            continue;
        }
        else {
            if (m_exit) {
                LogTrace("@@@ socket thread wakeup, exit thread @@@\n");
                break; //exit thread
            }

            if (FD_ISSET(m_connfd, &fdWrite)) {
                m_connected = true;
                FD_CLR(m_connfd, &fdWrite);
                LogTrace("### connect server {%s} success ###\n", getServerAddrInfo().c_str());
                NOTIFY(E_NET_CONNECT_SUCCESS, NULL);
            }
            else if (FD_ISSET(m_connfd, &fdRead)) {
                if (!m_connected) {
                    continue;
                }

                enum {
                    RECV_BUF_SIZE = 512 * 1024
                };
                char buf[RECV_BUF_SIZE];
                memset(buf, 0, sizeof(buf));

                int dataLen = sockets::read(m_connfd, buf, RECV_BUF_SIZE);
                if ((dataLen == 0 || dataLen == -1) && m_connected) {
                    m_connected = false;
                    int err = sockets::getNetError();
                    LogError("@@@ recv failed {ret: %d, err: %d}, exit thread @@@\n", dataLen, err);
                    NOTIFY(E_NET_PEER_CLOSED, &err);
                    break;
                }

                if (dataLen > 0 && m_callback.fn_recv) {
                    m_callback.fn_recv(m_connfd, buf, dataLen, m_callback.usrData);
                }
            }
        }
    }

    cleanup();
    LogTrace("=== exit socket thread ===\n");
    return 0;
}

void TcpClient::wakeup()
{
    if (m_connfd != INVALID_SOCKET) {
        sockets::close(m_connfd);
        m_connfd = INVALID_SOCKET;
    }
}

void TcpClient::cleanup()
{
    if (m_connfd != INVALID_SOCKET) {
        sockets::close(m_connfd);
        m_connfd = INVALID_SOCKET;
    }
}

string TcpClient::getServerAddrInfo()
{
    char buf[32] = {0};
    sockets::toHostPort(buf, 32, m_serverAddr.getSockAddrInet());
    return string(buf);
}

}//namespace BDK