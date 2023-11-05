#pragma once
#include "base.h"
#include <WinSock2.h>
#include <memory>
#pragma comment(lib , "rpcrt4.lib")

#pragma comment(lib , "ws2_32.lib")

class CAddress
{
public:
	CAddress()
	{
		m_port = -1;
		memset(&m_addr, 0, sizeof(sockaddr_in));
		m_addr.sin_family = AF_INET;
	}
	CAddress(const std::string& IP, short port)
	{
		m_ip = IP;
		m_port = port;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(port);
		m_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	}

	~CAddress() {}
	CAddress(const CAddress& addr)
	{
		m_port = addr.m_port;
		m_ip = addr.m_ip;
		memcpy(&m_addr, &addr.m_addr, sizeof(sockaddr_in));
	}
	CAddress& operator=(const CAddress& addr)
	{
		if (this != &addr) {
			m_port = addr.m_port;
			m_ip = addr.m_ip;
			memcpy(&m_addr, &addr.m_addr, sizeof(sockaddr_in));
		}
		return *this;
	}
	void Update(const std::string& IP, unsigned short port)
	{
		m_ip = IP;
		m_port = port;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(port);
		m_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	}

	void Update(unsigned short port)
	{
		m_addr.sin_port = htons(port);
		m_port = port;
	}

	operator sockaddr* ()const
	{
		return (sockaddr*)&m_addr;
	}

	operator sockaddr* ()
	{
		return (sockaddr*)&m_addr;
	}

	operator sockaddr_in* ()const
	{
		return (sockaddr_in*)&m_addr;
	}

	operator sockaddr_in* ()
	{
		return (sockaddr_in*)&m_addr;
	}

	int size()const { return sizeof(sockaddr_in); }

	void Fresh() {
		m_ip = inet_ntoa(m_addr.sin_addr);
	}

	std::string Ip()const { return m_ip; }
	unsigned short Port()const { return m_port; }
private:
	std::string m_ip;
	unsigned short m_port;
	sockaddr_in m_addr;
};




class Socket
{

public:
	Socket(bool bIsTcp = true)
	{
		m_socket = INVALID_SOCKET;
		if (bIsTcp) {
			m_socket = socket(PF_INET, SOCK_STREAM, 0);
		}
		else {
			m_socket = socket(PF_INET, SOCK_DGRAM, 0);
		}
	}

	

	Socket(SOCKET s) :m_socket(s) {}

	void Close()
	{
		if (m_socket != INVALID_SOCKET) {
			SOCKET temp = m_socket;
			m_socket = INVALID_SOCKET;
			closesocket(m_socket);
		}
	}

	operator SOCKET() { return m_socket; }

	~Socket() { Close(); }
private:
	SOCKET m_socket;
};

class CSocket
{
public:
	CSocket(bool bIsTcp = true) :m_sock(new Socket(bIsTcp)),m_bIsTcp(bIsTcp) {}
	~CSocket() { m_sock.reset(); }
	operator SOCKET()const { return *m_sock; }
	CSocket(const CSocket& sock) :m_sock(sock.m_sock),m_bIsTcp(sock.m_bIsTcp){}
	CSocket& operator=(const CSocket& sock) {
		if (&sock != this) {
			m_sock = sock.m_sock;
			m_bIsTcp = sock.m_bIsTcp;
		}
		return *this;
	}

	bool operator==(SOCKET sock)
	{
		return *m_sock == sock;
	}

	bool operator!=(SOCKET sock)
	{
		return !operator==(sock);
	}

	CSocket(SOCKET sock, bool isTcp) :m_sock(new Socket(sock)), m_bIsTcp(isTcp) {
		
	}

	int Bind(const CAddress& addr)
	{
		if (m_sock == nullptr) {
			m_sock.reset(new Socket(m_bIsTcp));
		}
		return bind((SOCKET)*m_sock, (sockaddr*)addr, addr.size());
	}

	int Listen(int backlog = 5) 
	{
		return listen(*m_sock, backlog);
	}

	CSocket Accept(CAddress& addr)
	{
		int size = addr.size();
		if (m_sock == nullptr)
			return CSocket(INVALID_SOCKET,true);
		SOCKET server = *m_sock;
		if (server == INVALID_SOCKET)
			return CSocket(INVALID_SOCKET, true);
		SOCKET s = accept(server, (sockaddr*)addr, &size);
		return CSocket(s, m_bIsTcp);
	}

	int Connect(const CAddress& addr)
	{
		return connect(*m_sock, addr, addr.size());
	}

	int Recv(CBuffer& buf) 
	{
		return recv(*m_sock, buf, buf.size(), 0);
	}

	int Send(const CBuffer& buf)
	{
		printf("%s\r\n", (char*)buf);
		int index = 0;
		char* pData = buf;
		int ret = 0;
		while (index < buf.size()) {
			ret = (int)send(*m_sock, pData + index, buf.size() - index, 0);
			if (ret < 0)
				return ret;
			if(ret == 0)
				break;
			index += ret;
		}
		return index;
		
	}

	int Close()
	{
		m_sock.reset();
		return 0;
	}
private:
	std::shared_ptr<Socket> m_sock;
	bool m_bIsTcp;
};

class SocketIniter
{

public:
	SocketIniter() {
		WSADATA wsa;
		int ret = WSAStartup(MAKEWORD(2, 2), &wsa);
	}

	~SocketIniter() {
		WSACleanup();
	}

};

