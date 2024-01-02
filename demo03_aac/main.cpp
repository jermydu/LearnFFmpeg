#include <iostream>
#include "format.h"
using namespace std;

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}

#define ADTS_HEADER_LEN     7

//采样率
const int samplingFrequencies[] = {
    96000,  // 0x0
    88200,  // 0x1
    64000,  // 0x2
    48000,  // 0x3
    44100,  // 0x4
    32000,  // 0x5
    24000,  // 0x6
    22050,  // 0x7
    16000,  // 0x8
    12000,  // 0x9
    11025,  // 0xa
    8000   // 0xb
    // 0xc d e f是保留的
};

int adts_header(char * const pAdtsHeader, const int dataLength,
                const int profile, const int samplerate,
                const int channels)
{

    int samplingFrequencyIndex = 3; // 默认使用48000hz
    int adtsLen = dataLength + 7;

    int frequenciesSize = sizeof(samplingFrequencies) / sizeof(samplingFrequencies[0]);
    int i = 0;
    for(i = 0; i < frequenciesSize; i++)
    {
        if(samplingFrequencies[i] == samplerate)
        {
            samplingFrequencyIndex = i;
            break;
        }
    }
    if(i >= frequenciesSize)
    {
        printf("unsupport samplerate:%d\n", samplerate);
        return -1;
    }

    /**
     * |1|1|1|1| |1|1|1|1| |1|1|1|1| |0|0|0|1|
    */
    pAdtsHeader[0] = 0xff;         //syncword:0xfff                          高8bits
    pAdtsHeader[1] = 0xf0;         //syncword:0xfff                          低4bits
    pAdtsHeader[1] |= (0 << 3);    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
    pAdtsHeader[1] |= (0 << 1);    //Layer:0                                 2bits
    pAdtsHeader[1] |= 1;           //protection absent:1                     1bit

    /**
     * |profile|profile|samplingFrequencyIndex|samplingFrequencyIndex|
     * |samplingFrequencyIndex|samplingFrequencyIndex|private_bit|channels|
    */
    pAdtsHeader[2] = (profile)<<6;            //profile:profile               2bits
    pAdtsHeader[2] |= (samplingFrequencyIndex & 0x0f)<<2; //sampling frequency index:samplingFrequencyIndex  4bits
    pAdtsHeader[2] |= (0 << 1);             //private bit:0                   1bit
    pAdtsHeader[2] |= (channels & 0x04)>>2; //channel configuration:channels  高1bit

    /**
     * |channels|channels|original|home|
     * |copyright id bit|copyright id start|frame length|frame length|
    */
    pAdtsHeader[3] = (channels & 0x03)<<6; //channel configuration:channels 低2bits
    pAdtsHeader[3] |= (0 << 5);               //original：0                1bit
    pAdtsHeader[3] |= (0 << 4);               //home：0                    1bit
    pAdtsHeader[3] |= (0 << 3);               //copyright id bit：0        1bit
    pAdtsHeader[3] |= (0 << 2);               //copyright id start：0      1bit
    pAdtsHeader[3] |= ((adtsLen & 0x1800) >> 11);           //frame length：value   高2bits

    pAdtsHeader[4] = (uint8_t)((adtsLen & 0x7f8) >> 3);     //frame length:value    中间8bits
    pAdtsHeader[5] = (uint8_t)((adtsLen & 0x7) << 5);       //frame length:value    低3bits
    pAdtsHeader[5] |= 0x1f;                                 //buffer fullness:0x7ff 高5bits
    pAdtsHeader[6] = 0xfc;      //‭11111100‬       //buffer fullness:0x7ff 低6bits
    // number_of_raw_data_blocks_in_frame：
    // 表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧。

    return 0;
}

void CloseAvFormatInput(AVFormatContext* pFormatContext)
{
    if(pFormatContext)
        avformat_close_input(&pFormatContext);
}

int main(int argc,char* argv[])
{
    cout << "demo03_aac" << endl;
    //打开网路流
    avformat_network_init();

    //输入视频文件
    const char* defaultFileName = "../testvideo/testvideo1.mp4";
    const char* outAAC = "../testvideo/testvideo1_aac.aac";
    char* inFileName = nullptr;
     FILE *pAACFd = nullptr;
    if(argv[1] == nullptr)
    {
        inFileName = (char* )defaultFileName;
    }else
    {
        inFileName = argv[1];
    }
    cout << "In FileName:" << inFileName << endl;

    //FormatContext
    AVFormatContext* pAvFormatContext = nullptr;
    int nAudioIndex = -1;  //音频索引

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

    AVPacket pAVPacket;
    av_init_packet(&pAVPacket);

    //av_find_best_stream 查找音频 index
    nAudioIndex = av_find_best_stream(pAvFormatContext,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
    if(nAudioIndex < 0)
    {
        char buf[1014] = {0};
        av_strerror(nAudioIndex,buf,sizeof(buf) - 1);
        std::string strError = util::Format("av_find_best_stream {0} failed {1}",inFileName,buf);
        cout << strError << endl;
        CloseAvFormatInput(pAvFormatContext);

        return -1;
    }

    //打印AAC级别
    cout << util::Format("audio profile:{0}, FF_PROFILE_AAC_LOW:{1}",pAvFormatContext->streams[nAudioIndex]
                         ->codecpar->profile,FF_PROFILE_AAC_LOW) << endl;

    if(pAvFormatContext->streams[nAudioIndex]->codecpar->codec_id != AV_CODEC_ID_AAC)
    {
        cout << util::Format("the media file no contain AAC stream, it's codec_id is {0}}",
               pAvFormatContext->streams[nAudioIndex]->codecpar->codec_id) << endl;
        CloseAvFormatInput(pAvFormatContext);

        return -1;
    }
    pAACFd = fopen(outAAC, "wb");
    if (!pAACFd)
    {
        cout << util::Format("Could not open destination file {0}", outAAC) << endl;
        return -1;
    }

    //读取媒体文件，并把aac数据帧写入到本地文件
    while(av_read_frame(pAvFormatContext, &pAVPacket) >=0 )
    {
        if(pAVPacket.stream_index == nAudioIndex)
        {
            char adts_header_buf[7] = {0};
            adts_header(adts_header_buf, pAVPacket.size,
                        pAvFormatContext->streams[nAudioIndex]->codecpar->profile,
                        pAvFormatContext->streams[nAudioIndex]->codecpar->sample_rate,
                        pAvFormatContext->streams[nAudioIndex]->codecpar->channels);
            // 写adts header , ts流不适用，ts流分离出来的packet带了adts header
            fwrite(adts_header_buf, 1, 7, pAACFd);
            // 写adts data
            int len = fwrite(pAVPacket.data, 1, pAVPacket.size, pAACFd);
            if(len != pAVPacket.size)
            {

                cout << util::Format("warning, length of writed data isn't equal pkt.size({0},{1})",
                       len,
                       pAVPacket.size) << endl;
            }
        }
        av_packet_unref(&pAVPacket);
    }

    CloseAvFormatInput(pAvFormatContext);
    if(pAACFd)
    {
       fclose(pAACFd);
    }

    return 0;
}
