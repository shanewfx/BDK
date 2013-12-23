#include <WinSock2.h> //included firstly, otherwise compile error

#include <stdio.h>
#include <tchar.h>
#include <signal.h>		// to catch Ctrl-C

#include <boost\bind.hpp>

#include "..\source\thread.h"
using BDK::thread;

#include "..\source\TcpSocket.h"
using BDK::TcpSocketCallback_t;
using BDK::TcpServer;
using BDK::TcpClient;
using BDK::sockets::InetAddress;

#ifdef _DEBUG
#pragma comment(lib, "..\\..\\bin\\BDK_d.lib")
#else
#pragma comment(lib, "..\\..\\bin\\BDK.lib")
#endif

TcpServer* g_server = NULL;

void recvData(SOCKET connfd, char* buf, int dataSize, void* usrData)
{
    printf("[server] --{connfd: %d} recv client msg-- %s\n", connfd, buf);
    TcpServer* server = (TcpServer*)usrData;
    string msg = "hi, I am server";
    server->send(connfd, msg.c_str(), msg.size());
}

void server_thread(void* usrData)
{
    printf("server_thread begin\n");
    TcpServer* server = (TcpServer*)usrData;
   
    InetAddress localAddr(58888);

    TcpSocketCallback_t callback;
    callback.fn_recv   = recvData;
    callback.fn_notify = NULL;
    callback.usrData   = server;
    server->setCallback(callback);

    server->start(localAddr);
    server->loop();

    printf("server_thread end\n");
}

void sigIntHandler(int sig)
{
    printf("Caught signal: %d, cleaning up, just a second...\n", sig);
    if (g_server) {
        g_server->stop();
    }

    // ignore all these signals now and let the connection close
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
}

int _tmain(int argc, _TCHAR* argv[])
{
    signal(SIGINT, sigIntHandler);
    signal(SIGTERM, sigIntHandler);

    TcpServer server;
    g_server = &server;

    thread serverthread(boost::bind(server_thread, &server), "serverthread");
    serverthread.start();

    printf("====== server is running ======\n");
#if 1
    serverthread.join();
#else
    int x;
    do {
        scanf("%d", &x);
        if (x == 0) {
            server.stop();
            break;
        }

    } while (x);
#endif
    printf("\n====== server is stopped ======\n");
    return 0;
}

