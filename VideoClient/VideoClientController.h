#pragma once

#include "CVlc.h"
#include "VideoClientDlg.h"

enum CVlcCommand {
	CVLC_PLAY,
	CVLC_PAUSE,
	CVLC_STOP,
	CVLC_GET_VOLUME,
	CVLC_GET_POSITION,
	CVLC_GET_LENGTH
};

class VideoClientController
{
public:
	VideoClientController() {
		m_dlg.m_controller = this;
	}
	~VideoClientController() {}
	int Init(CWnd*& pWnd);
	int Invoke();
	int SetHwnd(HWND hwnd);
	int SetVolume(int volume);
	int SetPosition(float position);
	float VideoControl(CVlcCommand cmd);
	// 如果路径中包含中文,则使用utf-8编码
	int SetMedia(const std::string& strUrl);
	VlcSize GetMediaInfo();
	std::string Unicode2Utf8(const std::wstring& strIn);
protected:
	CVlc m_vlc;
	CVideoClientDlg m_dlg;
	
};

