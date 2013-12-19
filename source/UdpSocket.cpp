#include <Winsock2.h>
#include "UdpSocket.h"
#include "log.h"

#pragma comment(lib, "Ws2_32.lib")

namespace BDK {

enum {
    RECV_BUF_SIZE = 512 * 1024
};


UdpSocket::UdpSocket()
    : m_inited(false)
    , m_socket(INVALID_SOCKET)
    , m_thread(NULL)
    , m_exit(false)
    , m_recvCallback(NULL)
    , m_userData(NULL)
{

}

UdpSocket::~UdpSocket()
{
	Close();
}

int UdpSocket::Initialize(unsigned short port)
{
    if (m_inited) {
        return 0;
    }

	WORD versionRequested = MAKEWORD(2, 2);

    WSADATA wsaData;
	WSAStartup(versionRequested, &wsaData);

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == m_socket) {
		return -1;
	}

	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family      = AF_INET;
	addr.sin_port        = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int retVal = bind(m_socket, (sockaddr *)&addr, sizeof(addr));
	if (retVal < 0) {
		closesocket(m_socket);
        return -1;
	}

    m_exit = false;
    m_thread = CreateThread(NULL, 0, ReceiveThreadProc, this, 0, NULL);
	if (NULL == m_thread) {
		return -1;
	}

    m_inited = true;
	return 0;
}

void UdpSocket::Close()
{
    if (!m_inited) {
        return;
    }

	if (m_thread != NULL) {		
        m_exit = true;
        LogTrace("=== wait udp recv thread exit begin ===\n");
        WaitForSingleObject(m_thread, INFINITE);
        LogTrace("=== wait udp recv thread exit end ===\n");
		CloseHandle(m_thread);
		m_thread = NULL;
	}

    if (m_socket != INVALID_SOCKET) {
	    closesocket(m_socket);
	    m_socket = INVALID_SOCKET;
    }

	WSACleanup();
    m_inited = false;
}

int UdpSocket::Send(char* targetAddr, unsigned short targetPort, const char* data, int len)
{
	if (INVALID_SOCKET == m_socket) {
		return -1;
	}

	sockaddr_in addr;
	addr.sin_family      = AF_INET;
	addr.sin_port        = htons(targetPort);
	addr.sin_addr.s_addr = inet_addr(targetAddr);

	return sendto(m_socket, data, len, 0, (SOCKADDR *)&addr, sizeof(addr));
}

int UdpSocket::Receive(char* data, int len, char* peerAddr, unsigned short* peerPort)
{
	if (INVALID_SOCKET == m_socket) {
		return -1;
	}

	sockaddr_in peerSocketAddr;
	int peerAddrSize = sizeof(peerSocketAddr);

	int retVal = recvfrom(m_socket, data, len, 0, (SOCKADDR *)&peerSocketAddr, &peerAddrSize);
	if (peerAddr) {
		strcpy(peerAddr, inet_ntoa(peerSocketAddr.sin_addr));
		*peerPort = ntohs(peerSocketAddr.sin_port);
	}
	
	return retVal;
}

int UdpSocket::SetReceiveCallback(RECV_CALLBACK recvCallback, void* userData)
{
	m_recvCallback = recvCallback;
    m_userData = userData;
	return 0;
}

int UdpSocket::ReceiveData()
{
    fd_set fdRead;

    while (!m_exit) {
	    FD_ZERO(&fdRead);	

	    FD_SET(m_socket, &fdRead); 

        timeval timeout;
        timeout.tv_sec  = 1;   
        timeout.tv_usec = 0;

	    int retVal = select(0, &fdRead, NULL, NULL, &timeout/*NULL*/);
	    if (SOCKET_ERROR == retVal)	{
		    if (m_socket != INVALID_SOCKET) {
			    closesocket(m_socket);
			    m_socket = INVALID_SOCKET;
		    }
		    return 1;
	    }
        else if (0 == retVal) {
            continue;
        }
        else {
            if (FD_ISSET(m_socket, &fdRead)) {
			    char buf[RECV_BUF_SIZE];
			    memset(buf, 0, sizeof(buf));

			    int len = Receive(buf, RECV_BUF_SIZE, NULL, NULL);
			    if (len > 0 && m_recvCallback) {
				    m_recvCallback(buf, len, m_userData);
			    }
		    }
	    }
    }

	return 0;
}

DWORD WINAPI UdpSocket::ReceiveThreadProc(LPVOID param)
{
	UdpSocket* udpClient = (UdpSocket*)param;
	if (NULL == udpClient) {
		return 1;
	}

	udpClient->ReceiveData();
    return 0;
}

}//namespace BDK