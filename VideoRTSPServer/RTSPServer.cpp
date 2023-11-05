#include "RTSPServer.h"

SocketIniter RTSPServer::m_initer;

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
	m_pool.Invoke();
	return 0;
}

void RTSPServer::Stop()
{
	m_sock.Close();
	m_threadMain.Stop();
	m_pool.Stop();
}

void RTSPServer::PlayCallBack(RTSPSession& session, RTSPServer* thiz)
{
	CAddress addr = session.GetClientUdpAddress();
	thiz->UdpWorker(addr);
}

int RTSPServer::threadWorker()
{
	CAddress addr;
	CSocket cli_sock = m_sock.Accept(addr);
	if (cli_sock != INVALID_SOCKET) {
		RTSPSession session(cli_sock);
		if (!m_lstSession.PushBack(session))
			printf("pushback err!\r\n");
		m_pool.DispatchWorker(ThreadWorker(this, FUNCTYPE(&RTSPServer::ThreadSession)));
	}
	return 0;
}

int RTSPServer::ThreadSession()
{
	RTSPSession session;
	if (m_lstSession.PopFront(session)) {
		int ret =  session.PickRequestAndReply(RTSPServer::PlayCallBack,this);
		return ret;
	}
	
	return -1;
}

int RTSPServer::UdpWorker(const CAddress& client)
{
	CBuffer buf = m_h264.ReadOneFrame();
	RTPFrame frame;
	while (buf.size() > 0) {
		m_helper.SendMediaFrame(frame, buf, client);
		buf = m_h264.ReadOneFrame();
	}
	return 0;
}

RTSPSession::RTSPSession()
{
	m_port = -1;
	UUID uuid;
	UuidCreate(&uuid);
	m_id.resize(8);
	sprintf((char*)m_id.c_str(), "%u%u", uuid.Data1,uuid.Data2);
}

RTSPSession::RTSPSession(const CSocket& sock):m_client(sock)
{
	m_port = -1;
	UUID uuid;
	UuidCreate(&uuid);
	m_id.resize(8);
	sprintf((char*)m_id.c_str(), "%u%u", uuid.Data1,uuid.Data2);
}

RTSPSession::RTSPSession(const RTSPSession& session)
{
	m_id = session.m_id;
	m_client = session.m_client;
	m_port = session.m_port;
}

RTSPSession& RTSPSession::operator=(const RTSPSession& session)
{
	if (this != &session) {
		m_id = session.m_id;
		m_client = session.m_client;
		m_port = session.m_port;
	}
	return *this;
}

int RTSPSession::PickRequestAndReply(RTSPPLAYCB cb, RTSPServer* thiz)
{
	int ret = -1;
	do {
		CBuffer buf = Pick();
		if (buf.size() <= 0) return -1;
		RTSPRequest req = AnalyseRequest(buf);
		if (req.method() < 0)
		{
			TRACE("buffer[%s]\r\n", (char*)buf);
			return -2;
		}

		RTSPReply reply = MakeReply(req);
		ret = m_client.Send(reply.toBuffer());
		if(req.method() == 2)
			m_port = atoi(req.port());
		if (req.method() == 3) {
			cb(*this, thiz);
		}
		
	} while (ret >= 0);
	return ret;
}

CAddress RTSPSession::GetClientUdpAddress() const
{
	CAddress addr;
	int len = addr.size();
	getsockname(m_client, addr, &len);
	addr.Fresh();
	addr.Update(m_port);
	return addr;
}

CBuffer RTSPSession::Pick()
{
	CBuffer result;
	int ret = 1;
	CBuffer buf(1);
	while (ret > 0) {
		buf.Zero();
		ret = m_client.Recv(buf);
		if (ret == -1)
			TRACE("wsa get last error : %d\r\n", WSAGetLastError());
		if (ret > 0) {
			result += buf;
			if (result.size() >= 4) {
			
				UINT tailVal = *(UINT*)((char*)result + result.size() - 4);
				if(tailVal == *(UINT*)"\r\n\r\n")
					break;
		
			}
		}
	}

	return result;
}

CBuffer RTSPSession::PickOneLine(CBuffer& buf)
{
	CBuffer line;
	int i = 0;
	for (; i < buf.size(); ++i) {
		line += buf.at(i);
		if(buf.at(i) == '\n') break;
	}
	CBuffer temp;
	if (i + 1 < buf.size()) {
		temp = (char*)buf + i + 1;
		buf = temp;
	}
	return line;
}

RTSPRequest RTSPSession::AnalyseRequest(const CBuffer& buf)
{
	TRACE("<%s>\r\n", (char*)buf);
	CBuffer data = buf;
	RTSPRequest req;
	if (buf.size() < 0) return req;
	CBuffer line = PickOneLine(data);
	CBuffer method(32), url(1024), version(16);
	if (sscanf(line, "%s %s %s \r\n", (char *)method, (char*)url, (char*)version) < 3) {
		TRACE("Error: [%s]\r\n",(char*)line);
		return req;
	}
	CBuffer seq(64);
	line = PickOneLine(data);
	if (sscanf(line, "CSeq: %s\r\n", (char*)seq) < 1) {
		TRACE("Error: [%s]\r\n", (char*)line);
		return req;
	}
	req.SetMethod(method);
	req.SetSequence(seq);
	req.SetUrl(url);
	//0 OPTIONS 1 DESCRIBE 2 SETUP 3 PLAY 4TEARDOWN
	if (strcmp(method, "OPTIONS") == 0 || strcmp(method, "DESCRIBE") == 0) {
		return req;
	}
	else if (strcmp(method, "SETUP") == 0){
		int port[2];
		do {
			line = PickOneLine(data);
			if(strstr((char *)line,"client_port") != NULL)break;
		} while (line.size() > 0);
		if (sscanf(line, "Transport: RTP/AVP;unicast;client_port=%d-%d\r\n", port, port + 1) == 2){
			req.SetClientPort(port);
			return req;
		}
	}
	else if (strcmp(method, "PLAY") == 0 || strcmp(method, "TEARDOWN") == 0) {
		CBuffer session(64);
		PickOneLine(data);
		line = PickOneLine(data);
		if (sscanf(line, "Session: %s\r\n", (char*)session) == 1) {
			req.SetSession(session);
		}
		return req;
	}

	return req;
}

RTSPReply RTSPSession::MakeReply(const RTSPRequest& req)
{
	RTSPReply reply;
	reply.SetSequence(req.sequence());
	if (req.session().size() > 0)
		reply.SetSession(req.session());
	else
		reply.SetSession(m_id);
	reply.SetMethod(req.method());
	switch (req.method())
	{
	case 0:		//	0 OPTIONS
		reply.SetOptions("Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY\r\n");
		break;
	case 1:		//	1 DESCRIBE
	{
		CBuffer sdp;
		sdp << "v=0\r\n";
		sdp << "o=-" << (char *)m_id << " 1 IN IP4 127.0.0.1\r\n";
		sdp << "t=0 0\r\n" << "a=control:*\r\n" << "m=video 0 RTP/AVP 96\r\n";
		sdp << "a=framerate:24\r\n";
		sdp << "a=rtpmap:96 H264/90000\r\n" << "a=control:track0\r\n";
		reply.SetSdp(sdp);
	}
		break;
	case 2:		//	2 SETUP
		reply.SetClientPort(req.port(), req.port(1));
		reply.SetServerPort("55000", "55001");
		reply.SetSession(m_id);
		break;
	case 3:		//	3 PLAY
		break;
	case 4:		//	4 TEARDOWN
		break;
	default:
		break;
	}

	return reply;
}

RTSPRequest::RTSPRequest()
{
	m_method = -1;
}

RTSPRequest::~RTSPRequest()
{
	m_method = -1;
}

RTSPRequest::RTSPRequest(const RTSPRequest& protocol)
{
	m_method = protocol.m_method;
	m_url = protocol.m_url;
	m_seq = protocol.m_seq;
	m_session = protocol.m_session;
	m_client_port[0] = protocol.m_client_port[0];
	m_client_port[1] = protocol.m_client_port[1];
}

RTSPRequest& RTSPRequest::operator=(const RTSPRequest& protocol)
{
	if (this != &protocol) {
		m_method = protocol.m_method;
		m_url = protocol.m_url;
		m_seq = protocol.m_seq;
		m_session = protocol.m_session;
		m_client_port[0] = protocol.m_client_port[0];
		m_client_port[1] = protocol.m_client_port[1];
	}
	return *this;
}

void RTSPRequest::SetMethod(const CBuffer& method)
{
	if (strcmp(method, "OPTIONS") == 0) m_method = 0;
	else if (strcmp(method, "DESCRIBE") == 0) m_method = 1;
	else if (strcmp(method, "SETUP") == 0) m_method = 2;
	else if (strcmp(method, "PLAY") == 0) m_method = 3;
	else if (strcmp(method, "TEARDOWN") == 0) m_method = 4;
}

void RTSPRequest::SetClientPort(int ports[])
{
	m_client_port[0] << ports[0];
	m_client_port[1] << ports[1];
}

void RTSPRequest::SetUrl(const CBuffer& url)
{
	m_url = url;
}

void RTSPRequest::SetSequence(const CBuffer& seq)
{
	m_seq = seq;
}

void RTSPRequest::SetSession(const CBuffer& session)
{
	m_session = session;
}

RTSPReply::RTSPReply()
{
	m_method = -1;
}

RTSPReply::~RTSPReply()
{
	m_method = -1;
}

RTSPReply::RTSPReply(const RTSPReply& reply)
{
	m_method = reply.m_method;
	m_client_port[0] = reply.m_client_port[0];
	m_client_port[1] = reply.m_client_port[1];
	m_server_port[0] = reply.m_server_port[0];
	m_server_port[1] = reply.m_server_port[1];
	m_seq = reply.m_seq;
	m_session = reply.m_session;
	m_sdp = reply.m_sdp;
}

RTSPReply& RTSPReply::operator=(const RTSPReply& reply)
{
	if (this != &reply) {
		m_method = reply.m_method;
		m_client_port[0] = reply.m_client_port[0];
		m_client_port[1] = reply.m_client_port[1];
		m_server_port[0] = reply.m_server_port[0];
		m_server_port[1] = reply.m_server_port[1];
		m_seq = reply.m_seq;
		m_session = reply.m_session;
		m_sdp = reply.m_sdp;
	}
	return *this;
}

CBuffer RTSPReply::toBuffer()
{
	CBuffer res;
	res << "RTSP/1.0 200 OK\r\n" << "CSeq:" << m_seq << "\r\n";
	switch (m_method)
	{
		case 0:		// 0 OPTIONS
			res << "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n\r\n";
			break;
		case 1:		// 1 DESCRIBE
			res << "Content-Base: 127.0.0.1\r\n";
			res << "Content-type: application/sdp\r\n";
			res << "Content-length: " << (int)m_sdp.size() << "\r\n\r\n";
			res << (char *)m_sdp;
			break; 
		case 2:		// 2 SETUP
			res << "Transport: RTP/AVP;unicast;client_port=" << m_client_port[0] <<"-"
				<< m_client_port[1] << ";server_port=" << m_server_port[0]<<"-" <<m_server_port[1]<<"\r\n";
			res << "Session: " << (char *)m_session << "\r\n\r\n";
			break;
		case 3:		// 3 PLAY
			res << "Range: npt=0.000-\r\n";
			res << "Session: " << (char *)m_session << "\r\n\r\n";
			break;
		case 4:		// 4 TEARDOWN
			res << "\r\n";
			break;
		default:
			break;
	}
	

	return res;
}

void RTSPReply::SetOptions(const CBuffer& options)
{
	m_options = (char *)options;
}

void RTSPReply::SetSequence(const CBuffer& seq)
{
	m_seq = (char*)seq;
}

void RTSPReply::SetSdp(const CBuffer& sdp)
{
	m_sdp = (char*)sdp;
}

void RTSPReply::SetClientPort(const CBuffer& port1, const CBuffer& port2)
{
	port1 >> m_client_port[0];
	port2 >> m_client_port[1];
}

void RTSPReply::SetServerPort(const CBuffer& port1, const CBuffer& port2)
{
	port1 >> m_server_port[0];
	port2 >> m_server_port[1];
}

void RTSPReply::SetSession(const CBuffer& session)
{
	m_session = (char *)session;
}

void RTSPReply::SetMethod(int method)
{
	m_method = method;
}
