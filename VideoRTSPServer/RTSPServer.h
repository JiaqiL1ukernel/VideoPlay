#pragma once
#include "Socket.h"
#include "EdoyunThread.h"
#include <map>

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
private:
	int method; //0 OPTIONS 1 DESCRIBE 2 SETUP 3 PLAY 4TEARDOWN
};

class RTSPServer
{
public:
	RTSPServer() :m_sock(true),m_status(0){}
	~RTSPServer();
	void Init(const std::string& strIP = "0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
protected:
	RTSPRequest AnalyseRequest(const std::string& data);
	int ThreadWorker();
	RTSPReply MakeReply(const RTSPRequest& req);
	int ThreadSession();
private:
	CSocket m_sock;
	int m_status; //0 未初始化 1 初始化完成 2 正在运行 3 结束
	EdoyunThread m_threadMain;
	EdoyunThreadPool m_pool;
	std::map<std::string, RTSPSession> m_mapSessions;
	SocketIniter m_initer;
};

