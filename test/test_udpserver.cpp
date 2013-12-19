#include <WinSock2.h> //included firstly, otherwise compile error

#include <stdio.h>
#include <tchar.h>
#include <string>
using std::string;

#include "..\source\UdpSocket.h"
using BDK::UdpSocket;

#define TARGET_IP    "127.0.0.1"
enum {
    HOST_PORT   = 28888,
    TARGET_PORT = 28889
};

#ifdef _DEBUG
#pragma comment(lib, "..\\..\\bin\\BDK_d.lib")
#else
#pragma comment(lib, "..\\..\\bin\\BDK.lib")
#endif

UdpSocket* g_server = NULL;

void CALLBACK recvData(char* buf, int dataSize, void* usrData)
{
    printf("[server] --recv udp client msg-- %s\n", buf);
    UdpSocket* server = (UdpSocket*)usrData;
    string msg = "hi, I am udp server";
    server->Send(TARGET_IP, TARGET_PORT, msg.c_str(), msg.size());
}

int _tmain(int argc, _TCHAR* argv[])
{
    UdpSocket server;
    g_server = &server;
    server.Initialize(HOST_PORT);
    server.SetReceiveCallback(recvData, &server);

    printf("====== server is running ======\n");

    int x;
    do {
        scanf("%d", &x);
        if (x == 0) {
            server.Close();
            break;
        }

    } while (x);

    printf("\n====== server is stopped ======\n");
    return 0;
}

