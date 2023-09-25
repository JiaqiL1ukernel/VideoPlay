#pragma once
#include <WinSock2.h>
#include <share.h>
#include <memory>



class CBuffer :public std::string
{
public:
	CBuffer(const char* str)
	{
		resize(strlen(str));
		memcpy((char*)c_str(), str, size());
	}

	CBuffer(size_t size = 0) :std::string()
	{
		if (size > 0) {
			resize(size);
			memset((void*)c_str(), 0, size);
		}
	}

	CBuffer(void* buffer, size_t size)
	{
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}

	~CBuffer()
	{
		std::string::~basic_string();
	}

	operator char* () { return (char*)c_str(); }
	operator const char* () { return c_str(); }
	operator byte* () { return (byte*)c_str(); }
	operator void* () { return (void*)c_str(); }
	void Update(void* buffer, size_t size)
	{
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
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
	operator SOCKET() { return *m_sock; }
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

	CSocket(SOCKET sock, bool isTcp) :m_sock(new Socket(sock)), m_bIsTcp(isTcp) {}

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
		SOCKET s = accept(*m_sock, (sockaddr*)addr, &size);
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
		
		return send(*m_sock, buf.c_str(), buf.size(), 0);
		
	}

	int Close()
	{
		m_sock.reset();
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
		WSAStartup(MAKEWORD(2, 2), &wsa);
	}

	~SocketIniter() {
		WSACleanup();
	}

};

