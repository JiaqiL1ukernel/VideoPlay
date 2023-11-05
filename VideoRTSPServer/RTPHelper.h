#pragma once
#include "base.h"
#include "Socket.h"
class RTPHeader
{
public:
	unsigned  short csrccount : 4;
	unsigned  short extension : 1;
	unsigned  short padding : 1;
	unsigned  short version : 2; 
	unsigned  short pytype : 7;
	unsigned  short mark : 1;
	unsigned short serial;
	unsigned timestamp;
	unsigned ssrc;
	unsigned csrc[15];
public:
	RTPHeader();
	RTPHeader(const RTPHeader& header);
	RTPHeader& operator=(const RTPHeader& header);
	operator CBuffer();
};

class RTPFrame
{
public:
	RTPHeader m_header;
	CBuffer m_pyload;
public:
	RTPFrame() {};
	operator CBuffer();
};

class RTPHelper
{

public:
	RTPHelper() :timestamp(0), m_udp(false) {
		m_udp.Bind(CAddress("0.0.0.0", 55000));
		m_file = fopen("correct.bin", "wb+");
	};
	~RTPHelper() { fclose(m_file); }
	int SendMediaFrame(RTPFrame& rtpFrame, CBuffer& frame,const CAddress& client);
private:
	int GetFrameSepSize(const CBuffer& frame);
	int SendFrame(const CBuffer& frame,const CAddress& client);

private:
	DWORD timestamp;
	CSocket m_udp;
	FILE* m_file;
};

