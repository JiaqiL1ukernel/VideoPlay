#include "RTPHelper.h"
#define MAX_RTPFRAMESIZE 1300
int RTPHelper::SendMediaFrame(RTPFrame& rtpFrame,CBuffer& frame, const CAddress& client)
{
    int sepSize = GetFrameSepSize(frame);
    size_t frameSize = frame.size() - sepSize;
    size_t restSize = frameSize % MAX_RTPFRAMESIZE;
    BYTE* pFrame = (BYTE*)frame + sepSize;
    int i;
    if (frameSize > MAX_RTPFRAMESIZE) {    //大包分片
        size_t count = frameSize / MAX_RTPFRAMESIZE;
        BYTE nalu = (pFrame[0] & 0x1F);
        for (i = 0; i < count; ++i) {
        //1.数据荷载部分
            rtpFrame.m_pyload.resize(MAX_RTPFRAMESIZE + 2);
            ((BYTE*)rtpFrame.m_pyload)[0] = 0x60 | 28; //第一个字节

            ((BYTE*)rtpFrame.m_pyload)[1] = nalu; //第二个字节  中间
            if(i == 0)
		        ((BYTE*)rtpFrame.m_pyload)[1] |= 0x80 ; //第二个字节  开始
            else if(restSize==0 && i==count-1)
		        ((BYTE*)rtpFrame.m_pyload)[1] |= 0x40 | nalu; //第二个字节  结束
            //真正的数据内容
            memcpy((BYTE*)rtpFrame.m_pyload + 2, pFrame + MAX_RTPFRAMESIZE * i+1, MAX_RTPFRAMESIZE);
            SendFrame(rtpFrame, client);
            rtpFrame.m_header.serial++;
        }
        if (restSize > 0) {
            //处理尾巴
            rtpFrame.m_pyload.resize(restSize + 2);
			((BYTE*)rtpFrame.m_pyload)[0] = 0x60 | 28; //第一个字节

			((BYTE*)rtpFrame.m_pyload)[1] = 0x40 | nalu; //第二个字节  
			memcpy((BYTE*)rtpFrame.m_pyload + 2, pFrame + MAX_RTPFRAMESIZE  * count+1, restSize - 1);
			SendFrame(rtpFrame, client);
            rtpFrame.m_header.serial++;
        }
    }
    else {  //小包发送
        rtpFrame.m_pyload.resize(frameSize);
        memcpy(rtpFrame.m_pyload, pFrame, frameSize);
		SendFrame(rtpFrame, client);
		rtpFrame.m_header.serial++;		
    }
    //时间戳从0开始美过一帧+90000(时钟频率)/24(每秒帧数)
    rtpFrame.m_header.timestamp += 90000 / 24;
    //发送后加入休眠，控制发送速度
    Sleep(1000 / 30);
    return 0;
}

int RTPHelper::GetFrameSepSize(const CBuffer& frame)
{
    BYTE buf[] = { 0,0,0,1 };
    if (memcmp(frame, buf, 4) == 0)return 4;
    return 3;
}

int RTPHelper::SendFrame(const CBuffer& frame, const CAddress& client)
{
    int ret = sendto(m_udp, frame, frame.size(), 0,client, client.size());
    fwrite(frame,1, frame.size(), m_file);
    BYTE buf[] = { 0,0,0,0,0,0,0,0 };
    fwrite(buf, 1, 8, m_file);
    TRACE("ret = %d  size = %d\n  IP = %s   port = %d\r\n",ret-12,frame.size()-12,client.Ip().c_str(),client.Port());
    return ret;
}

RTPHeader::RTPHeader()
{
    csrccount = 0;
    extension = 0;
    padding = 0;
    version = 2;
    pytype = 96;
    mark = 0;
    serial = 0;
    timestamp = 0;
    ssrc = 0x98765432;
    memset(csrc, 0, sizeof(csrc));
}

RTPHeader::RTPHeader(const RTPHeader& header)
{
    memset(csrc, 0, sizeof(csrc));
    size_t size = csrccount * 4 + 12;
    memcpy(this, &header, size);
}

RTPHeader& RTPHeader::operator=(const RTPHeader& header)
{
    if (this != &header) {
		memset(csrc, 0, sizeof(csrc));
		size_t size = csrccount * 4 + 12;
		memcpy(this, &header, size);
    }
    return *this;
}

RTPHeader::operator CBuffer()
{
    RTPHeader header(*this);
    header.serial = htons(header.serial);
    header.timestamp = htonl(header.timestamp);
    header.ssrc = htonl(header.ssrc);
    size_t size = 12 + csrccount * 4;
    CBuffer res(size);
    memcpy(res, &header, size);
    return res;
}



RTPFrame::operator CBuffer()
{
    CBuffer res;
    res += (CBuffer)m_header;
    res += m_pyload;
    return res;
}
