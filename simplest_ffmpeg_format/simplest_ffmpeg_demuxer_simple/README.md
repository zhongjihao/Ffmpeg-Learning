最简单的基于FFmpeg的视音频分离器（简化版）
Simplest FFmpeg Demuxer Simple


本程序将一个FLV封装的文件（其中视频编码为H.264，音频编码为MP3）分离成为两个文件：一个H.264编码的视频码流文件，一个MP3编码的音频码流文件。
需要注意的是，本程序是一个简单版的视音频分离器（Demuxer）。该分离器与原版的不同在于，没有初始化输出视频流和音频流的AVFormatContext。而是直接将解码后的得到的
AVPacket中的的数据通过fwrite()写入文件，但是缺点是并不适用于一些格式，对于MP3编码的音频是没有问题的。但是在分离MP4/FLV/MKV等一些格式中的AAC编码的码流的时候，得到的AAC码流是不能播放的。
原因是存储AAC数据的AVPacket的data字段中的数据是不包含7字节ADTS文件头的“砍头”的数据，是无法直接解码播放的（当然如果在这些数据前面手工加上7字节的ADTS文件头的话，就可以播放了）。


使用FFMPEG类库分离出多媒体文件中的音频码流
   在使用FFMPEG的类库进行编程的过程中，可以直接输出解复用之后的的音频数据码流。只需要在每次调用av_read_frame()之后将得到的音频的AVPacket存为本地文件即可。
   经试验，在分离MP3码流的时候，直接存储AVPacket即可。在分离AAC码流的时候，直接存储AVPacket后的文件可能是不能播放的。

   如果视音频复用格式是TS（MPEG2 Transport Stream），直接存储后的文件是可以播放的。
   复用格式是FLV，MP4等则不行。因为FLV，MP4这些属于“特殊容器”。经过仔细对比后发现，调用av_read_frame()后得到的AVPacket里面的内容是AAC纯数据，就是那种不包含ADTS文件头的AAC数据。
   因此如果想要得到可以播放的AAC文件，需要在每个AVPacket前面加上7字节ADTS文件头。



使用FFMPEG类库分离出多媒体文件中的H.264码流
   在使用FFMPEG的类库进行编程的过程中，可以直接输出解复用之后的的视频数据码流。只需要在每次调用av_read_frame()之后将得到的视频的AVPacket存为本地文件即可。
   在分离H.264码流的时候，直接存储AVPacket后的文件可能是不能播放的。

   如果视音频复用格式是TS（MPEG2 Transport Stream），直接存储后的文件是可以播放的。
   
   复用格式是FLV/MKV/MP4则不行。分离某些封装格式（例如MP4/FLV/MKV等）中的H.264的时候，需要首先写入SPS和PPS，否则会导致分离出来的数据
   没有SPS、PPS而无法播放,H.264的SPS和PPS信息存储在AVCodecContext的extradata中。需要使用ffmpeg中名称
   为“h264_mp4toannexb”的bitstream filter处理。有两种处理方式：
   1.使用bitstream filter处理每个AVPacket（简单）
     把每个AVPacket中的数据（data字段）经过bitstream filter“过滤”一遍。关键函数是av_bitstream_filter_filter()。
     把av_bitstream_filter_filter()的输入数据和输出数据（分别对应第4,5,6,7个参数）都设置成AVPacket的data字段就可以了。经过
     处理之后，AVPacket中的数据有如下变化：
	   每个AVPacket的data添加了H.264的NALU的起始码{0,0,0,1}
	   每个IDR帧数据前面添加了SPS和PPS
   
   2. 手工添加SPS，PPS（稍微复杂）
     将AVCodecContext的extradata数据经过bitstream filter处理之后得到SPS、PPS，拷贝至每个IDR帧之前。然后修改AVPacket的data，
     把前4个字节改为起始码，经过这两步也可以得到可以播放的H.264码流，相对于上面第一种方法来说复杂一些

   当封装格式为MPEG2TS的时候，不存在上述问题，在分离MPEG2码流的时候，直接存储AVPacket中data数据到本地文件即可播放
	
