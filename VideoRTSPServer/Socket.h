#pragma once
#include <WinSock2.h>
#include <share.h>
#include <memory>

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
	CSocket(bool bIsTcp) :m_sock(new Socket(bIsTcp)) {}
	~CSocket() { m_sock.reset(); }
	operator SOCKET() { return *m_sock; }
	CSocket(const CSocket& sock) :m_sock(sock.m_sock){}
	CSocket& operator=(const CSocket& sock) {
		if (&sock != this) m_sock = sock.m_sock;
		return *this;
	}

private:
	std::shared_ptr<Socket> m_sock;

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

