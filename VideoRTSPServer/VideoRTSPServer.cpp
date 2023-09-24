// VideoRTSPServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "socket.h"
#include "RTSPServer.h"
int main()
{
	RTSPServer sever;
	sever.Init();
	sever.Invoke();
	getchar();
	sever.Stop();
	return 0;
}
