#include <iostream>
#include "format.h"
using namespace std;

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}

void CloseAvFormatInput(AVFormatContext* pFormatContext)
{
    if(pFormatContext)
        avformat_close_input(&pFormatContext);
}

int main(int argc, char* argv[])
{
    cout << "06Decode" << endl;
    //打开网路流
    avformat_network_init();

    //输入视频文件
    const char* defaultFileName = "../../../../testvideo/testvideo1.mp4";
    char* inFileName = nullptr;
    if(argv[1] == nullptr)
    {
        inFileName = (char* )defaultFileName;
    }else
    {
        inFileName = argv[1];
    }
    cout << "In FileName:" << inFileName << endl;

    //FormatContext
    AVFormatContext* pAVFormatContext = nullptr;
    int nVideoIndex = -1;  //视频索引
    int nAudioIndex = -1;  //音频索引

    //参数设置
    AVDictionary* pOptions = nullptr;
    //设置rtsp流以tcp打开
    av_dict_set(&pOptions, "rtsp_transport", "tcp", 0);
    //设置网络延迟时间
    av_dict_set(&pOptions, "max_delay", "500", 0);


    //打开输入流，读取头信息 必须调用 avformat_close_input 关闭
    int ret = avformat_open_input(&pAVFormatContext,inFileName,nullptr,nullptr);
    if(ret < 0)
    {
        char buf[1014] = {0};
        av_strerror(ret,buf,sizeof(buf) - 1);
        std::string strError = util::Format("open {0} failed {1}",inFileName,buf);
        cout << strError << endl;
        return -1;
    }

    //读取媒体文件流信息
    ret = avformat_find_stream_info(pAVFormatContext,nullptr);
    if(ret < 0)
    {
        char buf[1014] = {0};
        av_strerror(ret,buf,sizeof(buf) - 1);
        std::string strError = util::Format("avformat_find_stream_info {0} failed {1}",inFileName,buf);
        cout << strError << endl;
        CloseAvFormatInput(pAVFormatContext);
        return -1;
    }

    //打印输入输出格式详细信息
    cout << util::Format("=================Print detailed information about {0}",inFileName) << endl;
    av_dump_format(pAVFormatContext, 0, inFileName, 0);
    cout << "=================\n" << endl;

    //自己从avformatContex中取数据
    cout << "=================avformat_find_stream_info" << endl;
    //路径名/文件名
    cout << util::Format("media name-->[{0}]",pAVFormatContext->url) << endl;
    //流媒体数量
    cout << util::Format("stream number-->[{0}]",pAVFormatContext->nb_streams) << endl;
    //媒体文件码率 单位bps
    cout << util::Format("media bitrate-->[{0}kbps]",pAVFormatContext->bit_rate/1024) << endl;
    //duration  微妙->秒
    int totalSeconds = pAVFormatContext->duration / AV_TIME_BASE;
    int hour = totalSeconds / 3600;
    int minute = (totalSeconds % 3600) / 60;
    int second = totalSeconds % 60;
    cout << util::Format("media duration-->[{0}:{1}:{2}]",hour,minute,second) << endl;
    cout << "=================\n" << endl;

    cout << "=================AVStream Info" << endl;
    //遍历方式读取视频信息和音频信息
    for(uint32_t i = 0; i < pAVFormatContext->nb_streams; ++i)
    {
        //获取一个流
        AVStream* pAVStream = pAVFormatContext->streams[i];
        //音频流
        if(AVMEDIA_TYPE_AUDIO == pAVStream->codecpar->codec_type)
        {
            nAudioIndex = i;
            cout << util::Format("---AVStream:Audio index:[{0}]",pAVStream->index) << endl;
            //音频编解码器的采样率(每秒钟的采样数)  Hz
            cout << util::Format("Audio samplerate->[{0}Hz]",pAVStream->codecpar->sample_rate) << endl;
            //音频采样格式
            if(AV_SAMPLE_FMT_FLTP == pAVStream->codecpar->format)
            {
                cout << util::Format("Audio sample format->[{0}]","AV_SAMPLE_FMT_FLTP") << endl;
            }else if(AV_SAMPLE_FMT_S16P == pAVStream->codecpar->format)
            {
                cout << util::Format("Audio sample format->[{0}]","AV_SAMPLE_FMT_S16P") << endl;
            }else
            {
                cout << util::Format("Audio sample format->[{0}]",pAVStream->codecpar->format) << endl;
            }
            //音频通道数
            cout << util::Format("Audio channel number->[{0}]",pAVStream->codecpar->ch_layout.nb_channels) << endl;
            //音频压缩编码格式
            if(AV_CODEC_ID_AAC == pAVStream->codecpar->codec_id)
            {
                cout << util::Format("Audio codec->[{0}]","AAC") << endl;
            }else if(AV_CODEC_ID_MP3 == pAVStream->codecpar->codec_id)
            {
                cout << util::Format("Audio codec->[{0}]","MP3") << endl;
            }else
            {
                cout << util::Format("Audio codec->[{0}]",pAVStream->codecpar->codec_id) << endl;
            }
            //一帧单通道样本数
            cout << util::Format("Audio frame size->[{0}]", pAVStream->codecpar->frame_size);
            //帧率 fps = sample_rate / frame_size
            //音频总时长 秒
            if(pAVStream->duration != AV_NOPTS_VALUE)
            {
                //ffmpeg中的内部计时单位（时间基），ffmepg中的所有时间都是于它为一个单位，
                //比如AVStream中的duration，即这个流的长度为duration个AV_TIME_BASE.
                int durationAudio = (pAVStream->duration) * av_q2d(pAVStream->time_base);
                cout << util::Format("Audio duration->[{0}:{1}:{2}]",durationAudio / 3600, (durationAudio % 3600) / 60, (durationAudio % 60)) << endl;
            }
            else
            {
                cout << util::Format("Audio duration unknown") << endl;
            }
        }
        //视频流
        else if(AVMEDIA_TYPE_VIDEO == pAVStream->codecpar->codec_type)
        {
            nVideoIndex = i;
            cout << util::Format("---AVStream:Video index:[{0}]",pAVStream->index) << endl;
            //视频帧率 fps 表示每秒出现多少帧
            cout << util::Format("Video fps->[{0}fps]",av_q2d(pAVStream->avg_frame_rate)) << endl;
            //视频压缩编码格式
            if(AV_CODEC_ID_MPEG4 == pAVStream->codecpar->codec_id)
            {
                cout << util::Format("Video codec->[{0}]","MPEG4") << endl;
            }
            else if(AV_CODEC_ID_H264 == pAVStream->codecpar->codec_id)
            {
                cout << util::Format("Video codec->[{0}]","H264") << endl;
            }
            else
            {
                cout << util::Format("Video codec->[{0}]",pAVStream->codecpar->codec_id) << endl;
            }

            //视频帧宽高
            cout << util::Format("Video width->[{0}] height->[{1}]",pAVStream->codecpar->width,
                                 pAVStream->codecpar->height) << endl;

            //视频总时长 秒
            if(pAVStream->duration != AV_NOPTS_VALUE)
            {
                //ffmpeg中的内部计时单位（时间基），ffmepg中的所有时间都是于它为一个单位，
                //比如AVStream中的duration，即这个流的长度为duration个AV_TIME_BASE.
                int durationVideo = (pAVStream->duration) * av_q2d(pAVStream->time_base);
                cout << util::Format("Video duration->[{0}:{1}:{2}]",durationVideo / 3600, (durationVideo % 3600) / 60, (durationVideo % 60)) << endl;
            }
            else
            {
                cout << util::Format("Audio duration unknown") << endl;
            }
        }
        //字幕流
        else if(AVMEDIA_TYPE_SUBTITLE == pAVStream->codecpar->codec_type)
        {
            cout << util::Format("---AVStream:SubTitle index:[{0}]",pAVStream->index) << endl;
        }
    }
    //av_find_best_stream 也可获取流索引号
    nVideoIndex = av_find_best_stream(pAVFormatContext,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
    nAudioIndex = av_find_best_stream(pAVFormatContext,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
    cout << "=================\n" << endl;

    ///视频解码器
    //找到视频解码器
    const AVCodec* pVideoCodec = avcodec_find_decoder(pAVFormatContext->streams[nVideoIndex]->codecpar->codec_id);
    if (pVideoCodec == nullptr)
    {
        std::string strError = util::Format("avcodec_find_decoder failed {0}", pAVFormatContext->streams[nVideoIndex]->codecpar->codec_id);
        cout << strError << endl;
         
        return -1;
    }
    cout << util::Format("find video avcodec: {0}", pAVFormatContext->streams[nVideoIndex]->codecpar->codec_id) << endl;
    //创建解码器上下文
    AVCodecContext* pVideoCodecContext = avcodec_alloc_context3(pVideoCodec);
    //配置解码器上下文参数 从codec中复制参数
    avcodec_parameters_to_context(pVideoCodecContext, pAVFormatContext->streams[nVideoIndex]->codecpar);
    pVideoCodecContext->thread_count = 8;   //八线程解码

    //打开解码器
    ret = avcodec_open2(pVideoCodecContext,nullptr,nullptr);
    if (ret < 0)
    {
        char buf[1014] = { 0 };
        av_strerror(ret, buf, sizeof(buf) - 1);
        std::string strError = util::Format("video avcodec_open2 failed {0}", buf);
        cout << strError << endl;
        return -1; 
    }
    cout << util::Format("video avcodec_open2 success") << endl;


    ///音频解码器
    //找到音频解码器
    const AVCodec* pAudioCodec = avcodec_find_decoder(pAVFormatContext->streams[nAudioIndex]->codecpar->codec_id);
    if (pAudioCodec == nullptr)
    {
        std::string strError = util::Format("avcodec_find_decoder failed {0}", pAVFormatContext->streams[nAudioIndex]->codecpar->codec_id);
        cout << strError << endl;

        return -1;
    }
    cout << util::Format("find audio avcodec: {0}", pAVFormatContext->streams[nAudioIndex]->codecpar->codec_id) << endl;
    //创建解码器上下文
    AVCodecContext* pAudioCodecContext = avcodec_alloc_context3(pAudioCodec);
    //配置解码器上下文参数 从codec中复制参数
    avcodec_parameters_to_context(pAudioCodecContext, pAVFormatContext->streams[nAudioIndex]->codecpar);
    pAudioCodecContext->thread_count = 8;   //八线程解码

    //打开解码器
    ret = avcodec_open2(pAudioCodecContext, nullptr, nullptr);
    if (ret < 0)
    {
        char buf[1014] = { 0 };
        av_strerror(ret, buf, sizeof(buf) - 1);
        std::string strError = util::Format("audio avcodec_open2 failed {0}", buf);
        cout << strError << endl;
        return -1;
    }
    cout << util::Format("audio avcodec_open2 success") << endl;


    //read AVPacket
    int pktCurCount = 0;
    const int printPktMaxCount = 20;
    AVPacket* pAVPacket = av_packet_alloc();
    AVFrame* pAVFrame = av_frame_alloc();
    cout << "=================av_read_frame" << endl;
    while(1)
    {
        //malloc AVPacket 并初始化
        ret = av_read_frame(pAVFormatContext,pAVPacket);
        if(ret < 0)
        {
            cout << util::Format("av_read_frame end") << endl;
            break;
        }
        AVCodecContext* pAVCodecContext = nullptr;
        //if(++pktCurCount < printPktMaxCount)
        {
            //音频
            if(pAVPacket->stream_index == nAudioIndex)
            {
                cout << util::Format("audio pts-->[{0}]",pAVPacket->pts) << endl;
                cout << util::Format("audio dts-->[{0}]",pAVPacket->dts) << endl;
                cout << util::Format("audio size-->[{0}]",pAVPacket->size) << endl;
                cout << util::Format("audio pos-->[{0}]",pAVPacket->pos) << endl;
                cout << util::Format("audio duration ms-->[{0}]",
                                     pAVPacket->duration * av_q2d(pAVFormatContext->streams[nAudioIndex]->time_base) * 1000) << endl;
                
                pAVCodecContext = pAudioCodecContext;
            }
            //视频
            if(pAVPacket->stream_index == nVideoIndex)
            {
                cout << util::Format("video pts-->[{0}]",pAVPacket->pts) << endl;
                cout << util::Format("video pts ms-->[{0}]", pAVPacket->pts * av_q2d(pAVFormatContext->streams[nVideoIndex]->time_base) * 1000) << endl;

                cout << util::Format("video dts-->[{0}]",pAVPacket->dts) << endl;
                cout << util::Format("video size-->[{0}]",pAVPacket->size) << endl;
                cout << util::Format("video pos-->[{0}]",pAVPacket->pos) << endl;
                cout << util::Format("video duration ms-->[{0}]",
                                     pAVPacket->duration * av_q2d(pAVFormatContext->streams[nVideoIndex]->time_base) * 1000) << endl;
                
                pAVCodecContext = pVideoCodecContext;
            }
            else
            {
                cout << util::Format("unkonwn streamIndex-->[{0}]",pAVPacket->stream_index) << endl;
            }
        }

        //解码视频
        if (pAVCodecContext)
        {
            //发送packet到解码线程 不占用CPU （如何取出缓冲，发送nullptr pAVPacket然后receive多次）
            ret = avcodec_send_packet(pAVCodecContext, pAVPacket);
            //释放 引用计数-1 为0 释放空间
            av_packet_unref(pAVPacket);
            if (ret < 0)
            {
                /*char buf[1014] = { 0 };
                av_strerror(ret, buf, sizeof(buf) - 1);
                std::string strError = util::Format("avcodec_send_packet failed {0}", buf);
                cout << strError << endl;*/
                continue;
            }

            while (1)
            {
                //从线程中获取解码结果 不占用CPU 一次send可能对应多次receive
                ret = avcodec_receive_frame(pAVCodecContext, pAVFrame); 
                if (ret < 0)
                    break;
                cout << util::Format("receive_frame format->[{0}] linesize->[{1}]", pAVFrame->format, pAVFrame->linesize[0]) << endl;
            }
        }
        else
        {
            //释放 引用计数-1 为0 释放空间
            av_packet_unref(pAVPacket);
        }
        
       
    }

    if (pAVFrame)
        av_frame_free(&pAVFrame);
    if(pAVPacket)
        av_packet_free(&pAVPacket);

    CloseAvFormatInput(pAVFormatContext);
    if (pVideoCodecContext)
        avcodec_free_context(&pVideoCodecContext);
    if (pAudioCodecContext)
        avcodec_free_context(&pAudioCodecContext);
    return 0;
}






















