#include "MediaFile.h"

MediaFile::MediaFile()
	:m_file(NULL),m_type(-1)
{

}

MediaFile::~MediaFile()
{
	Close();
}

int MediaFile::Open(const CBuffer& path, int type)
{
	m_file = fopen((char*)path, "rb");
	if (m_file == NULL) {
		// TODO: ¥ÌŒÛÃ· æ
		return -1;
	}
	m_type = type;
	fseek(m_file, 0, SEEK_END);
	m_tail = ftell(m_file);
	Reset();
	return 0;
}

CBuffer MediaFile::ReadOneFrame()
{
	switch (m_type)
	{
	case 96:
		return ReadH264Frame();
	default:
		break;
	}
	return CBuffer();
}

void MediaFile::Close()
{
	m_type = -1;
	if (m_file != NULL) {
		FILE* fp = m_file;
		m_file = NULL;
		fclose(fp);
	}
}

void MediaFile::Reset()
{
	if (m_file) {
		fseek(m_file, 0, SEEK_SET);
	}
}

long MediaFile::FindH264Head()
{
	while (!feof(m_file)) {
		char c = 0x7F;
		while (!feof(m_file)) {
			c = fgetc(m_file);
			if (c == 0)break;
		}
		if (!feof(m_file)) {
			c = fgetc(m_file);
			if (c == 0) {
				c = fgetc(m_file);
				if (c == 1)
					return ftell(m_file) - 3;
				else if (c == 0) {
					c = fgetc(m_file);
					if (c == 1)
						return ftell(m_file) - 4;
				}
			}
		}
	}
	return -1;
}

CBuffer MediaFile::ReadH264Frame()
{
	if (m_file) {
		long off = FindH264Head();
		fseek(m_file, off + 3, SEEK_SET);
		long tail = FindH264Head();
		if (tail == -1) tail = m_tail;
		long size = tail - off;
		fseek(m_file, off, SEEK_SET);
		CBuffer res(size);
		fread(res, 1,size , m_file);
		return res;
	}
}
