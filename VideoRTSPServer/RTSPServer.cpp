#include "RTSPServer.h"

RTSPServer::~RTSPServer()
{
	Stop();
}

int RTSPServer::Init(const std::string& strIP, short port)
{
	m_addr.Update(strIP, port);
	m_sock.Bind(m_addr);
	m_sock.Listen();
	return 0;
}

int RTSPServer::Invoke()
{
	m_threadMain.Start();
	return 0;
}

void RTSPServer::Stop()
{
	m_sock.Close();
	m_threadMain.Stop();
	m_pool.Stop();
}

int RTSPServer::threadWorker()
{
	CAddress addr;
	CSocket cli_sock = m_sock.Accept(addr);
	if (cli_sock != INVALID_SOCKET) {
		m_clients.PushBack(cli_sock);
		m_pool.DispatchWorker(ThreadWorker(this, FUNCTYPE(&RTSPServer::ThreadSession)));
	}
	return 0;
}

int RTSPServer::ThreadSession()
{
	CSocket client;//TODO:能否拿到?
	CBuffer buf(1024 * 16);
	int len = client.Recv(buf);
	if (len <= 0) {
		//TODO:清理掉client
		return -1;
	}
	RTSPRequest req = AnalyseRequest(buf);
	RTSPReply ack = MakeReply(req);
	
	client.Send(ack.toBuffer());
	return 0;
}
