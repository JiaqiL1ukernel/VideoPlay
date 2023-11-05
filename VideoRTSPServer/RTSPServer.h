#pragma once
#include "CEdoyunQueue.h"
#include "Socket.h"
#include <map>
#include <rpc.h>
#include "RTPHelper.h"
#include "MediaFile.h"

#pragma comment(lib , "rpcrt4.lib")


class RTSPRequest
{
public:
	RTSPRequest();
	~RTSPRequest();
	RTSPRequest(const RTSPRequest&);
	RTSPRequest& operator= (const RTSPRequest&);
	void SetMethod(const CBuffer& method);
	void SetClientPort(int ports[]);
	void SetUrl(const CBuffer& url);
	void SetSequence(const CBuffer& seq);
	void SetSession(const CBuffer& session);
	int method()const { return m_method; }
	const CBuffer& url()const { return m_url; }
	const CBuffer& session()const { return m_session; };
	const CBuffer& sequence()const { return m_seq; }
	const CBuffer& port(int index = 0)const { return index? m_client_port[1] : m_client_port[0]; }
private:
	int m_method; //-1 初始化 0 OPTIONS 1 DESCRIBE 2 SETUP 3 PLAY 4TEARDOWN
	CBuffer m_seq;
	CBuffer m_url;
	CBuffer m_session;
	CBuffer m_client_port[2];
};


class RTSPReply
{
public:
	RTSPReply();
	~RTSPReply();
	RTSPReply(const RTSPReply& );
	RTSPReply& operator= (const RTSPReply& );
	CBuffer toBuffer();
	void SetOptions(const CBuffer& options);
	void SetSequence(const CBuffer& seq);
	void SetSdp(const CBuffer& sdp);
	void SetClientPort(const CBuffer& port1, const CBuffer& port2);
	void SetServerPort(const CBuffer& port1, const CBuffer& port2);
	void SetSession(const CBuffer& session);
	void SetMethod(int method);

private:
	int m_method; //0 OPTIONS 1 DESCRIBE 2 SETUP 3 PLAY 4TEARDOWN
	int m_client_port[2];
	int m_server_port[2];
	CBuffer m_seq;
	CBuffer m_options;
	CBuffer m_session;
	CBuffer m_sdp;
};


class RTSPSession;
class RTSPServer;
typedef void (*RTSPPLAYCB)(RTSPSession& session, RTSPServer* thiz);
class RTSPSession
{
public:
	RTSPSession();
	RTSPSession(const CSocket& sock);
	RTSPSession(const RTSPSession& session);
	RTSPSession& operator=(const RTSPSession& session);
	~RTSPSession() {}
	int PickRequestAndReply(RTSPPLAYCB cb, RTSPServer* thiz);
	CAddress GetClientUdpAddress()const;
private:
	CBuffer Pick();
	CBuffer PickOneLine(CBuffer& buf);
	RTSPRequest AnalyseRequest(const CBuffer& buf);
	RTSPReply MakeReply(const RTSPRequest& req);
private:
	CBuffer m_id;
	CSocket m_client;
	unsigned short m_port;
};

class RTSPServer
{
public:
	RTSPServer() :m_sock(true), m_status(0),m_pool(10) {
		m_threadMain.UpdateWorker
		(ThreadWorker(this, (FUNCTYPE)&RTSPServer::threadWorker));
		m_h264.Open("./test.h264");
	}
	~RTSPServer();
	int Init(const std::string& strIP = "0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
	static void PlayCallBack(RTSPSession& session, RTSPServer* thiz);
protected:
	
	int threadWorker();
	int ThreadSession();
	int UdpWorker(const CAddress& addr);
private:
	static SocketIniter m_initer;
	CSocket m_sock;
	CAddress m_addr;
	int m_status; //0 未初始化 1 初始化完成 2 正在运行 3 结束
	EdoyunThread m_threadMain;
	EdoyunThreadPool m_pool;
	//std::map<std::string, RTSPSession> m_mapSessions;
	CEdoyunQueue<RTSPSession> m_lstSession;
	RTPHelper m_helper;
	MediaFile m_h264;
};

