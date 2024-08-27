// HelloFFmepg.cpp: 定义应用程序的入口点。
//

#include <iostream>

using namespace std;
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
}

int main()
{
    std::cout << av_version_info() << std::endl;
    std::cout << avcodec_configuration() << std::endl;
    return 0;
}

