#pragma once
#ifdef _WIN32
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#endif
#include "vlc.h"
#include <string>

struct VlcSize
{
	int nWidth;
	int nHeight;
	VlcSize(int width = 0, int height = 0) {
		nWidth = width;
		nHeight = height;
	}

	VlcSize(const VlcSize& size) {
		nWidth = size.nWidth;
		nHeight = size.nHeight;
	}

	VlcSize& operator=(const VlcSize& size)
	{
		if (this != &size) {
			nWidth = size.nWidth;
			nHeight = size.nHeight;
		}
		return *this;
	}
};

class CVlc
{
public:
	CVlc();
	~CVlc();
	int SetHwnd(HWND hwnd);
	int SetMedia(const std::string& strUrl);
	int Play();
	int Pause();
	int Stop();
	float GetPosition();
	int SetPosition(float pos);
	int GetVolume();
	int SetVolume(int volume);
	float GetLength();
	VlcSize GetMediaInfo();
	std::string Unidcode2Utf8(const std::wstring strIn);

protected:
	libvlc_instance_t* m_instance;
	libvlc_media_t* m_media;
	libvlc_media_player_t* m_player;
	std::string m_url;
	HWND m_hwnd;
};

