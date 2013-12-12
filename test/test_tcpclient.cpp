#include <stdio.h>
#include <tchar.h>
#include <Winsock2.h>

#include "\BDK\source\TcpSocket.h"
using BDK::TcpSocketCallback_t;
using BDK::TcpClient;
using BDK::sockets::InetAddress;

#ifdef _DEBUG
#pragma comment(lib, "\\BDK\\bin\\BDK_d.lib")
#else
#pragma comment(lib, "\\BDK\\bin\\BDK.lib")
#endif

void recvData(char* buf, int dataSize, void* usrData)
{
    printf("[client] --recv server msg-- %s\n", buf);
}

void netNotify(uint32_t eventCode, void* param, void* usrData)
{
    printf("[netNotify] event: %d -> %s\n", eventCode, BDK::getEventDescriptor(eventCode));
}

int _tmain(int argc, _TCHAR* argv[])
{
    TcpClient client;

    TcpSocketCallback_t callback;
    callback.fn_recv   = recvData;
    callback.fn_notify = netNotify;
    callback.usrData   = NULL;
    client.setCallback(callback);

    InetAddress serverAddr("127.0.0.1", 58888);
    client.connect(serverAddr);

    int i = 60;
    while (i > 0) {
        i--;
        Sleep(1000);
        string msg = "hi, I am client";
        client.send(msg.c_str(), msg.size());
    }

    client.disconnect();
    return 0;
}
