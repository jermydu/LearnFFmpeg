#项目类型
TEMPLATE = app
#console 程序
CONFIG += console c++11

#生成exe 目录
DESTDIR = $$PWD/../Bin

#源文件
SOURCES += HelloFFmepg.cpp

#include
INCLUDEPATH += $$PWD/../ffmpeg-6.0-full_build-shared/include
#lib
LIBS +=   $$PWD/../ffmpeg-6.0-full_build-shared/lib/avcodec.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/avdevice.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/avfilter.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/avformat.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/avutil.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/postproc.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/swresample.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/swscale.lib \

#Qmake中打印信息
message($$QMAKESPEC)
