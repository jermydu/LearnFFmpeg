# CMakeList.txt: HelloFFmepg 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# include
include_directories("..\\dependents\\ffmpeg-6.0-full_build-shared\\include\\")
include_directories("..\\dependents\\format\\")

# lib
link_directories("..\\dependents\\ffmpeg-6.0-full_build-shared\\lib\\")
link_libraries("avcodec.lib")
link_libraries("avdevice.lib")
link_libraries("avfilter.lib")
link_libraries("avformat.lib")
link_libraries("avutil.lib")
link_libraries("postproc.lib")
link_libraries("swresample.lib")
link_libraries("swscale.lib")
# 将源代码添加到此项目的可执行文件。
add_executable (02Demux "main.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 02Demux PROPERTY CXX_STANDARD 20)
endif()

