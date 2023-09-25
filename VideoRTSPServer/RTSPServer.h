#pragma once
#include "Socket.h"
#include "EdoyunThread.h"
#include "CEdoyunQueue.h"
#include <map>


class CAddress 
{
public:
	CAddress()
	{
		m_port = -1;
		memset(&m_addr, 0, sizeof(sockaddr_in));
		m_addr.sin_family = AF_INET;
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
	void Update(const std::string& IP, short port)
	{
		m_ip = IP;
		m_port = port;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(port);
		m_addr.sin_addr.S_un.S_addr = inet_addr(IP.c_str());
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
		return (sockaddr_in*) & m_addr;
	}

	operator sockaddr_in* ()
	{
		return (sockaddr_in*)&m_addr;
	}

	int size()const { return sizeof(sockaddr_in); }
private:
	std::string m_ip;
	short m_port;
	sockaddr_in m_addr;
};

class RTSPSession
{
public:
	RTSPSession();
	RTSPSession(const RTSPSession& session);
	RTSPSession& operator=(const RTSPSession& session);
	~RTSPSession();
};

class RTSPRequest
{
public:
	RTSPRequest();

	~RTSPRequest();
	RTSPRequest(const RTSPRequest& );
	RTSPRequest& operator= (const RTSPRequest& );
private:
	int method; //0 OPTIONS 1 DESCRIBE 2 SETUP 3 PLAY 4TEARDOWN
};

class RTSPReply
{
public:
	RTSPReply();
	~RTSPReply();
	RTSPReply(const RTSPReply& );
	RTSPReply& operator= (const RTSPReply& );
	CBuffer toBuffer();
private:
	int method; //0 OPTIONS 1 DESCRIBE 2 SETUP 3 PLAY 4TEARDOWN
};

class RTSPServer
{
public:
	RTSPServer() :m_sock(true), m_status(0),m_pool(10) {
		m_threadMain.UpdateWorker
		(ThreadWorker(this, (FUNCTYPE)&RTSPServer::threadWorker));
	}
	~RTSPServer();
	int Init(const std::string& strIP = "0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
protected:
	RTSPRequest AnalyseRequest(const std::string& data);
	int threadWorker();
	RTSPReply MakeReply(const RTSPRequest& req);
	int ThreadSession();
private:
	CSocket m_sock;
	CAddress m_addr;
	int m_status; //0 未初始化 1 初始化完成 2 正在运行 3 结束
	EdoyunThread m_threadMain;
	EdoyunThreadPool m_pool;
	std::map<std::string, RTSPSession> m_mapSessions;
	SocketIniter m_initer;
	CEdoyunQueue<CSocket> m_clients;
};

