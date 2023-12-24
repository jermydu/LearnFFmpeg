// HelloFFmepg.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavutil/avutil.h"
}

int main(int argc, char* argv[])
{
    std::cout << av_version_info() << std::endl;
    std::cout << avcodec_configuration() << std::endl;
}
