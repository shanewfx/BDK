#ifndef _UDP_SOCKET_H_
#define _UDP_SOCKET_H_

#include <boost\noncopyable.hpp>

namespace BDK {

typedef void (CALLBACK* RECV_CALLBACK)(char* data, int len, void* userData);

class UdpSocket : boost::noncopyable
{
public:
	UdpSocket();
   ~UdpSocket();

	int  Initialize(unsigned short port);
	void Close();

    int  SetReceiveCallback(RECV_CALLBACK recvCallback, void* userData);

	int  Send(char* targetAddr, unsigned short targetPort, const char* data, int len);
	int  Receive(char* data, int len, char* peerAddr, unsigned short* peerPort);
	
private:
    static DWORD WINAPI ReceiveThreadProc(LPVOID param);
	int ReceiveData();

    bool   m_inited;
    bool   m_exit;
    HANDLE m_thread;
	SOCKET m_socket;

	RECV_CALLBACK m_recvCallback;
    void*  m_userData;
};

}//namespace BDK

#endif//_UDP_SOCKET_H_
