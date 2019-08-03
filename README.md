Ffmpeg在Linux下的编译步骤 \
   一、下载FFmpeg \
       git clone https://git.ffmpeg.org/ffmpeg.git \
   二、安装依赖包 \
       sudo apt-get update \
       sudo apt-get upgrade \
       sudo apt-get update -qq && sudo apt-get -y install autoconf automake build-essential cmake git \
       libass-dev libfreetype6-dev libsdl2-dev libtheora-dev libtool libva-dev libvdpau-dev libvorbis-dev \
       libxcb1-dev libxcb-shm0-dev libxcb-xfixes0-dev mercurial pkg-config texinfo wget zlib1g-dev
       
   sudo apt-get install nasm \
   sudo apt-get install yasm \
   sudo apt-get install libx264-dev \
   sudo apt-get install libx265-dev libnuma-dev -y \
   sudo apt-get install libvpx-dev \
   sudo apt-get install libfdk-aac-dev\
   sudo apt-get install libmp3lame-dev \
   sudo apt-get install libopus-dev \
   sudo apt-get install libspeex-dev -y

 三、配置Ffmpeg \
 cd ffmpeg/ \
 执行\
 ./configure --prefix=/usr/local/ffmpeg --enable-gpl --enable-nonfree --enable-libfdk-aac --enable-libx264 --enable-libx265 --enable-filter=delogo --enable-debug --disable-optimizations --enable-opengl --enable-libspeex --enable-libopus --enable-libmp3lame --enable-shared --enable-pthreads

 四、编译安装 \
 make \
 sudo make install

 五、查看FFMpeg版本是否安装成功 \
 ffmpeg -version



simplest_ffmpeg_streamer工程包含如下部分 \
 最简单的基于FFmpeg的推流器（以推送RTMP为例) \
 最简单的基于FFMPEG的推流器附件：收流器

 simplest_ffmpeg_format工程包含如下部分 \
 最简单的基于FFmpeg的封装格式处理：视音频分离器简化版（demuxer-simple）\
 最简单的基于FFmpeg的封装格式处理：视音频分离器（demuxer）\
 最简单的基于FFmpeg的封装格式处理：视音频复用器（muxer）\
 最简单的基于FFMPEG的封装格式处理：封装格式转换（remuxer）

 simplest_ffmpeg_demo工程包含如下部分 \
 1 Ffmpeg文件目录基本操作
 2 从MP4或flv中分离出h264视频和aac或MP3音频



FFMPEG 简单操作命令

湖南卫视   rtmp://58.200.131.2:1935/livetv/hunantv
CCTV1   http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8


FFmpeg发送流媒体的命令（UDP，RTP，RTMP）
1. UDP
1.1. 发送H.264裸流至组播地址
注：组播地址指的范围是224.0.0.0—239.255.255.255

下面命令实现了发送H.264裸流“test.h264”至地址udp://233.233.233.223:6666

ffmpeg -re -i test.h264 -vcodec copy -f h264 udp://233.233.233.223:6666
注1：-re一定要加，代表按照帧率发送，否则ffmpeg会一股脑地按最高的效率发送数据。

注2：-vcodec copy要加，否则ffmpeg会重新编码输入的H.264裸流。

1.2. 播放承载H.264裸流的UDP
ffplay -f h264 udp://233.233.233.223:6666
注：需要使用-f说明数据类型是H.264

播放的时候可以加一些参数，比如-max_delay，下面命令将-max_delay设置为100ms：

ffplay -max_delay 100000 -f h264 udp://233.233.233.223:6666


1.3. 发送MPEG2裸流至组播地址
下面的命令实现了读取本地摄像头的数据，编码为MPEG2，发送至地址udp://233.233.233.223:6666。

ffmpeg -re -i test.h264 -vcodec mpeg2video -f mpeg2video udp://233.233.233.223:6666
1.4.  播放MPEG2裸流
指定-vcodec为mpeg2video即可。

ffplay -vcodec mpeg2video udp://233.233.233.223:6666


2. RTP
2.1. 发送H.264裸流至组播地址。
下面命令实现了发送H.264裸流“test.h264”至地址rtp://233.233.233.223:6666

ffmpeg -re -i test.h264 -vcodec copy -f rtp rtp://233.233.233.223:6666 > test.sdp
注1：-re一定要加，代表按照帧率发送，否则ffmpeg会一股脑地按最高的效率发送数据。

注2：-vcodec copy要加，否则ffmpeg会重新编码输入的H.264裸流。

注3：最右边的“>test.sdp”用于将ffmpeg的输出信息存储下来形成一个sdp文件。该文件用于RTP的接收。
当不加“>test.sdp”的时候，ffmpeg会直接把sdp信息输出到控制台。将该信息复制出来保存成一个后缀是.sdp文本文件，也是可以用来接收该RTP流的。
加上“>test.sdp”后，可以直接把这些sdp信息保存成文本。

2.2. 播放承载H.264裸流的RTP。
ffplay -protocol_whitelist "file,http,https,rtp,udp,tcp,tls" -i test.sdp
 

3.RTMP
推流 
    ffmpeg -re -i out.mp4 -c:a copy -c:v copy -f flv rtmp://localhost:1935/liveApp/room
	
拉流
    ffmpeg -i rtmp://localhost:1935/liveApp/room -c copy -f flv dump.flv
    或 ffplay rtmp://localhost:1935/liveApp/room


录制屏幕（mkv格式）：
ffmpeg -f x11grab -r 30 -i :0.0 -f alsa -i hw:0,0 -acodec flac -vcodec ffvhuff out.mkv

录制屏幕（无声音）：
ffmpeg -f x11grab -s wxga -r 25 -i :0.0 -sameq out.mpg

录音：
ffmpeg -f alsa -ac 2 -i hw:0,0 out.avi

录音（arecord）：
arecord -D plughw:0,0 -f S16_LE -c 2 -r 22050 out.wav

录像：
ffmpeg -f alsa -i default -f v4l2 -s 640x480 -i /dev/video0 out.mpg

录像（MKV格式）：
ffmpeg -f alsa -i default -f v4l2 -s 1024x768 -i /dev/video0 -acodec flac -vcodec ffvhuff out.mkv

录像（无声音）：
ffmpeg -f v4l2 -s 640x480 -i /dev/video0 out.mpg


分解与复用命令
ffmpeg -i 道路偏离+前车碰撞视频.mp4 -vcodec copy -acodec copy out.flv

//只获取视频
ffmpeg -i 道路偏离+前车碰撞视频.mp4 -vcodec copy -an out.h264 

//提取yuv数据
ffmpeg -i 道路偏离+前车碰撞视频.mp4 -an -c:v rawvideo -pix_fmt yuv420p out.yuv
-c:v rawvideo 指定将视频转成原始数据
-pix_format yuv420p 指定转换格式为yuv420p


//提取pcm数据
ffmpeg -i 道路偏离+前车碰撞视频.mp4 -vn -ar 44100 -ac 2 -f s16le out.pcm
-ar:指定音频采样率 44100 即44.1KHz
-ac:指定音频声道channel 2 为双声道
-f：数据存储格式 s：Signed 有符号的， 16： 每一个数值用16位表示， l： little， e： end

播放pcm文件
ffplay -ar 44100 -ac 2 -f s16le out.pcm

Ffmpeg滤镜

视频裁剪
ffmpeg -i 道路偏离+前车碰撞视频.mp4 -vf crop=in_w-200:in_h-200 -c:v libx264 -c:a copy out.mp4
crop 滤镜


从输入视频开始播放的时刻裁剪,裁剪时长10秒
ffmpeg -i 道路偏离+前车碰撞视频.mp4 -ss 00:00:00 -t 10 out.ts


视频拼接,多个视频拼接为一个视频，多个视频列表如1.ts,2.ts存放在input.txt
input.txt格式为
file '1.ts'
file '2.ts'
ffmpeg -f concat -i input.txt out.flv


视频转图片
ffmpeg -i out.flv -r 1 -f image2 image-%3d.jpeg
-r 帧率 每秒多少张图片

多张图片转视频
ffmpeg -i image-%3d.jpeg test.mp4




