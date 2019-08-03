/*************************************************************************
    > File Name: simplest_ffmpeg_demuxer_simple.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com

 * This software split a media file (in Container such as 
 * MKV, FLV, AVI...) to video and audio bitstream.
 * In this example, it demux a FLV file to H.264 bitstream
 * and MP3 bitstream.
 * Note:
 * This is a simple version of "Simplest FFmpeg Demuxer". It is 
 * more simple because it doesn't init Output Video/Audio stream's
 * AVFormatContext. It write AVPacket's data to files directly.
 * The advantages of this method is simple. The disadvantages of
 * this method is it's not suitable for some kind of bitstreams. For
 * example, AAC bitstream in FLV/MP4/MKV Container Format(data in
 * AVPacket lack of 7 bytes of ADTS header).

 ************************************************************************/

#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavutil/log.h>
#include <libavformat/avformat.h>

#ifdef __cplusplus
};
#endif


//'1': Use H.264 Bitstream Filter 
#define USE_H264BSF 1


#define ADTS_HEADER_LEN  7;

void adts_header(char *szAdtsHeader, int dataLen){

    int audio_object_type = 2; // AAC LC
    int sampling_frequency_index = 0xb; //采样率的下标 0x4代表44100,0x8代表16000,0x3代表48000,0x2代表64000,0xb代表8000
    int channel_config = 2; //声道数，比如2表示立体声双声道

    int adtsLen = dataLen + 7;

    szAdtsHeader[0] = 0xff;         //syncword:0xfff                          高8bits
    szAdtsHeader[1] = 0xf0;         //syncword:0xfff                          低4bits
    szAdtsHeader[1] |= (0 << 3);    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
    szAdtsHeader[1] |= (0 << 1);    //Layer:0                                 2bits 
    szAdtsHeader[1] |= 1;           //protection absent:1                     1bit

    szAdtsHeader[2] = (audio_object_type - 1)<<6;            //profile:audio_object_type - 1                      2bits
    szAdtsHeader[2] |= (sampling_frequency_index & 0x0f)<<2; //sampling frequency index:sampling_frequency_index  4bits 
    szAdtsHeader[2] |= (0 << 1);                             //private bit:0                                      1bit
    szAdtsHeader[2] |= (channel_config & 0x04)>>2;           //channel configuration:channel_config               高1bit

    szAdtsHeader[3] = (channel_config & 0x03)<<6;     //channel configuration:channel_config      低2bits
    szAdtsHeader[3] |= (0 << 5);                      //original：0                               1bit
    szAdtsHeader[3] |= (0 << 4);                      //home：0                                   1bit
    szAdtsHeader[3] |= (0 << 3);                      //copyright id bit：0                       1bit  
    szAdtsHeader[3] |= (0 << 2);                      //copyright id start：0                     1bit
    szAdtsHeader[3] |= ((adtsLen & 0x1800) >> 11);           //frame length：value   高2bits

    szAdtsHeader[4] = (uint8_t)((adtsLen & 0x7f8) >> 3);     //frame length:value    中间8bits
    szAdtsHeader[5] = (uint8_t)((adtsLen & 0x7) << 5);       //frame length:value    低3bits
    szAdtsHeader[5] |= 0x1f;                                 //buffer fullness:0x7ff 高5bits
    szAdtsHeader[6] = 0xfc;
}


int main(int argc, char* argv[])
{
	AVFormatContext *ifmt_ctx = NULL;
	AVPacket pkt;
	int ret, i;
	int videoindex = -1,audioindex = -1;
	const char *in_filename = NULL;
  const char *out_filename_v = NULL;
	const char *out_filename_a = NULL;

	av_log_set_level(AV_LOG_DEBUG);
  
	if(argc < 4){
		av_log(NULL,AV_LOG_ERROR,"the count of params should be more than four\n");
    return -1;
  }

  in_filename = argv[1];
  out_filename_v = argv[2];
	out_filename_a = argv[3];
  
	av_register_all();

	//Input
	if((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
		av_log(NULL,AV_LOG_ERROR,"Could not open input file %s.\n",in_filename);
		return -1;
	}

	if((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		av_log(NULL,AV_LOG_ERROR,"Failed to retrieve input stream information\n");
		return -1;
	}

	av_log(NULL,AV_LOG_DEBUG,"ifmt_ctx->nb_streams: %d\n",ifmt_ctx->nb_streams);
	for(i=0; i<ifmt_ctx->nb_streams; i++) {
		if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			videoindex = i;
		}else if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			audioindex = i;
		}
	}
	
	//Dump Format------------------
	av_log(NULL,AV_LOG_DEBUG,"\n=========Input file===================\n");
	av_dump_format(ifmt_ctx, 0, in_filename, 0);
	av_log(NULL,AV_LOG_DEBUG,"\n==========================\n");

	FILE *fp_audio = fopen(out_filename_a,"w+");  
	FILE *fp_video = fopen(out_filename_v,"w+");  

	/*
	FIX: H.264 in some container format (FLV, MP4, MKV etc.) need 
	"h264_mp4toannexb" bitstream filter (BSF)
	  *Add SPS,PPS in front of IDR frame
	  *Add start code ("0,0,0,1") in front of NALU
	H.264 in some container (MPEG2TS) don't need this BSF.
	*/
#if USE_H264BSF
	AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb"); 
#endif

	while(av_read_frame(ifmt_ctx, &pkt) >= 0){
		if(pkt.stream_index == videoindex){
#if USE_H264BSF

			av_log(NULL,AV_LOG_DEBUG,"1 Write Video Packet. size:%d\tpts:%lld,  data[0]: %x, %x ,%x ,%x, sps=%x\n",pkt.size,pkt.pts,pkt.data[0],pkt.data[1],pkt.data[2],pkt.data[3],pkt.data[4]);
			av_bitstream_filter_filter(h264bsfc, ifmt_ctx->streams[videoindex]->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
			av_log(NULL,AV_LOG_DEBUG,"2 Write Video Packet. size:%d\tpts:%lld,  data[0]: %x, %x ,%x ,%x, sps=%x\n",pkt.size,pkt.pts,pkt.data[0],pkt.data[1],pkt.data[2],pkt.data[3],pkt.data[4]);
			fwrite(pkt.data,1,pkt.size,fp_video);
		}else if(pkt.stream_index == audioindex){
			/*
			AAC in some container format (FLV, MP4, MKV etc.) need to add 7 Bytes
			ADTS Header in front of AVPacket data manually.
			Other Audio Codec (MP3...) works well.
			*/
		  av_log(NULL,AV_LOG_DEBUG,"Write Audio Packet. size:%d\tpts:%lld\n",pkt.size,pkt.pts);
			// char adts_header_buf[7];
      // adts_header(adts_header_buf, pkt.size);
      // fwrite(adts_header_buf, 1, 7, fp_audio);
			fwrite(pkt.data,1,pkt.size,fp_audio);
		}
		av_free_packet(&pkt);
	}

#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);  
#endif

	fclose(fp_video);
	fclose(fp_audio);

	avformat_close_input(&ifmt_ctx);

	if(ret < 0 && ret != AVERROR_EOF) {
		av_log(NULL,AV_LOG_ERROR,"Error occurred.\n");
		return -1;
	}
	return 0;
}


