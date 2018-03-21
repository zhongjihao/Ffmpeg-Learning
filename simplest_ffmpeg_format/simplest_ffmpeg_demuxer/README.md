最简单的基于FFmpeg的视音频分离器 \
Simplest FFmpeg Demuxer

本程序可以将封装格式中的视频码流数据和音频码流数据分离出来。在该例子中， 将MPEG2TS的文件分离得到H.264视频码流文件和AAC音频码流文件。

编译执行 \
 make 
./demuxer

播放h264文件使用mpv Media Player播放器或使用命令播放ffplay -stats -f h264 cuc_ieschool.h264，VLC播放器有问题，播放会出现严重卡顿

