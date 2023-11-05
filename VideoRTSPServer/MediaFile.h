#pragma once
#include "base.h"

class MediaFile
{
public:
	MediaFile();
	~MediaFile();

	int Open(const CBuffer& path,int type = 96);
	//如果Cbuffer.size()==0，说明没有帧了
	CBuffer ReadOneFrame();
	void Close();
	//重置后,ReadOneFrame又会有值返回
	void Reset();

private:
	long FindH264Head();
	CBuffer ReadH264Frame();
private:
	FILE* m_file;
	long m_tail;
	CBuffer m_filepath;
	//96 H264
	int m_type;
};

