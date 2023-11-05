// VideoPlay.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#ifdef _WIN32
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#endif
#include <iostream>
#include "vlc.h"
#include <Windows.h>

std::string Unicode2Utf8(const std::wstring wstr)
{
    std::string str;
    int len = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    str.resize(len + 1);
    ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), (LPSTR)str.c_str(), len, NULL, NULL);
    return str;

}

int main()
{
    int argc = 1;
    char* argv[2];
    argv[0] = (char *)"--ignore-config";
    libvlc_instance_t* vlc_ins = libvlc_new(argc, argv);
    std::string path = Unicode2Utf8(L"911Mothers_2010W-480p.mp4");
    libvlc_media_t* media = libvlc_media_new_path(vlc_ins, path.c_str());
    libvlc_media_player_t *player  = libvlc_media_player_new_from_media(media);
    do {
        int ret = libvlc_media_player_play(player);
        if (ret == -1) {
            printf("play failed!\n");
            break;
        }

        libvlc_time_t tm = libvlc_media_player_get_length(player);
        printf("%02d:%02d:%02d\n", int(tm / 3600), int((tm / 60) % 60), int(tm % 60));
        getchar();
        libvlc_media_player_pause(player);
        getchar();
        libvlc_media_player_play(player);
        getchar();
        libvlc_media_player_stop(player);


    } while (0);
    libvlc_media_player_release(player);
    libvlc_media_release(media);
    libvlc_release(vlc_ins);

}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
