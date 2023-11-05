// VideoRTSPServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//



#include "RTSPServer.h"
#include "Socket.h"
#include "RTPHelper.h"
int main()
{


	RTSPServer sever;
	sever.Init();
	sever.Invoke();
	printf("Press any key to stop\r\n");
	getchar();
	sever.Stop();
	return 0;
	
}
