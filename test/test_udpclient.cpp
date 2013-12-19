#include <stdio.h>
#include <tchar.h>
#include <string>
using std::string;

#include <Winsock2.h>

#include "..\source\UdpSocket.h"
using BDK::UdpSocket;

#define TARGET_IP    "127.0.0.1"
enum {
    HOST_PORT   = 28889,
    TARGET_PORT = 28888
};

#ifdef _DEBUG
#pragma comment(lib, "..\\..\\bin\\BDK_d.lib")
#else
#pragma comment(lib, "..\\..\\bin\\BDK.lib")
#endif


void CALLBACK recvData(char* buf, int dataSize, void* usrData)
{
    printf("[client] --recv udp server msg-- %s\n", buf);
}

int _tmain(int argc, _TCHAR* argv[])
{
    UdpSocket client;
    client.Initialize(HOST_PORT);
    client.SetReceiveCallback(recvData, &client);

    int i = 60;
    while (i > 0) { 
        i--;
        Sleep(1000);
        string msg = "hi, I am udp client";
        client.Send(TARGET_IP, TARGET_PORT, msg.c_str(), msg.size());
    }

    client.Close();
    return 0;
}
