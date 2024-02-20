TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

#生成exe 目录
DESTDIR = $$PWD/../Bin

#include
INCLUDEPATH += $$PWD/../ffmpeg-6.0-full_build-shared/include \
            += $$PWD/../dependents/format
#lib
LIBS +=   $$PWD/../ffmpeg-6.0-full_build-shared/lib/avcodec.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/avdevice.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/avfilter.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/avformat.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/avutil.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/postproc.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/swresample.lib \
          $$PWD/../ffmpeg-6.0-full_build-shared/lib/swscale.lib \
