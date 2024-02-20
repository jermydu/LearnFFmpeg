#include <iostream>
#include "format.h"
using namespace std;

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavformat/avio.h>
    #include <libavcodec/bsf.h>
    #include <libavutil/log.h>
}

void CloseAvFormatInput(AVFormatContext* pFormatContext)
{
    if(pFormatContext)
        avformat_close_input(&pFormatContext);
}

int main(int argc,char* argv[])
{
    cout << "demo04_nalu" << endl;
    //打开网路流
    avformat_network_init();

    //输入视频文件
    const char* defaultFileName = "../testvideo/testvideo1.mp4";
    const char* defaultOutFile = "../testvideo/testvideo1.h264";
    char* inFileName = (char* )defaultFileName;;
    char* outFileName = (char* )defaultOutFile;

    cout << "In FileName:" << inFileName << endl;
    cout << "Out FileName:" << outFileName << endl;

    FILE* pOutFile = fopen(outFileName,"wb");

    //FormatContext
    AVFormatContext* pAvFormatContext = avformat_alloc_context();
    if(!pAvFormatContext)
    {
        cout << "[error] Could not allocate context." << endl;
    }

    //打开输入流，读取头信息 必须调用 avformat_close_input 关闭
    int ret = avformat_open_input(&pAvFormatContext,inFileName,nullptr,nullptr);
    if(ret < 0)
    {
        char buf[1014] = {0};
        av_strerror(ret,buf,sizeof(buf) - 1);
        std::string strError = util::Format("open {0} failed {1}",inFileName,buf);
        cout << strError << endl;
        CloseAvFormatInput(pAvFormatContext);
        return -1;
    }

    //读取媒体文件流信息
    ret = avformat_find_stream_info(pAvFormatContext,nullptr);
    if(ret < 0)
    {
        char buf[1014] = {0};
        av_strerror(ret,buf,sizeof(buf) - 1);
        std::string strError = util::Format("avformat_find_stream_info {0} failed {1}",inFileName,buf);
        cout << strError << endl;
        CloseAvFormatInput(pAvFormatContext);
        return -1;
    }

    //打印输入输出格式详细信息
    cout << util::Format("=================Print detailed information about {0}",inFileName) << endl;
    av_dump_format(pAvFormatContext, 0, inFileName, 0);
    cout << "=================\n" << endl;

    //av_find_best_stream 查找视频 index
    int nVideoIndex = av_find_best_stream(pAvFormatContext,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
    if(nVideoIndex < 0)
    {
        char buf[1014] = {0};
        av_strerror(nVideoIndex,buf,sizeof(buf) - 1);
        std::string strError = util::Format("av_find_best_stream {0} failed {1}",inFileName,buf);
        cout << strError << endl;
        CloseAvFormatInput(pAvFormatContext);

        return -1;
    }

    //avpacket
    AVPacket* pAVPacket = av_packet_alloc();
    av_init_packet(pAVPacket);

    // 1 获取相应的比特流过滤器
    //FLV/MP4/MKV等结构中，h264需要h264_mp4toannexb处理。添加SPS/PPS等信息。
    // FLV封装时，可以把多个NALU放在一个VIDEO TAG中,结构为4B NALU长度+NALU1+4B NALU长度+NALU2+...,
    // 需要做的处理把4B长度换成00000001或者000001
    const AVBitStreamFilter* pBsfilter = av_bsf_get_by_name("h264_mp4toannexb");
    AVBSFContext* pBsfCtx = nullptr;
    // 2 初始化过滤器上下文
    av_bsf_alloc(pBsfilter, &pBsfCtx); //AVBSFContext;
    // 3 添加解码器属性
    avcodec_parameters_copy(pBsfCtx->par_in, pAvFormatContext->streams[nVideoIndex]->codecpar);
    av_bsf_init(pBsfCtx);

    //读取文件
    int iFileEnd = 0;   //文件是否读取结束
    while(0 == iFileEnd)
    {
        //没有更多包可读
        if((ret = av_read_frame(pAvFormatContext,pAVPacket)) < 0)
        {
            iFileEnd = 1;
            std::string strError = util::Format("av_read_frame file end:ret {0} ",ret);
            cout << strError << endl;
        }
        //读取到数据
        if(0 == ret && pAVPacket->stream_index == nVideoIndex)
        {
            int inputSize = pAVPacket->size;

            if (av_bsf_send_packet(pBsfCtx, pAVPacket) != 0) // bitstreamfilter内部去维护内存空间
            {
                av_packet_unref(pAVPacket);   // 你不用了就把资源释放掉
                continue;       // 继续送
            }
            av_packet_unref(pAVPacket);   // 释放资源
            while(av_bsf_receive_packet(pBsfCtx, pAVPacket) == 0)
            {
                size_t size = fwrite(pAVPacket->data, 1, pAVPacket->size, pOutFile);
                if(size != pAVPacket->size)
                {
                    std::string strError = util::Format("fwrite failed-> write: {0} pkt_size: {1} ",size, pAVPacket->size);
                    cout << strError << endl;
                }
                av_packet_unref(pAVPacket);
            }

            // TS流可以直接写入
//            size_t size = fwrite(pAVPacket->data, 1, pAVPacket->size, pOutFile);
//            if(size != pAVPacket->size)
//            {
//                std::string strError = util::Format("fwrite failed-> write: {0} pkt_size: {1} ",size, pAVPacket->size);
//                cout << strError << endl;
//            }
//            av_packet_unref(pAVPacket);
        }
        else
        {
            if(0 == ret)
            {
                av_packet_unref(pAVPacket);
            }
        }
    }

    if(pOutFile)
        fclose(pOutFile);
    if(pBsfCtx)
        av_bsf_free(&pBsfCtx);
    if(pAVPacket)
        av_packet_free(&pAVPacket);

    CloseAvFormatInput(pAvFormatContext);
    cout << "Finish" << endl;
    return 0;
}
