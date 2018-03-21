/*************************************************************************
    > File Name: simplest_ffmpeg_demuxer_simple.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2018年03月20日 星期二 17时08分38秒
 ************************************************************************/

#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavformat/avformat.h>
#ifdef __cplusplus
};
#endif


//'1': Use H.264 Bitstream Filter 
#define USE_H264BSF 1

int main(int argc, char* argv[])
{
	AVFormatContext *ifmt_ctx = NULL;
	AVPacket pkt;
	int ret, i;
	int videoindex = -1,audioindex = -1;
	const char *in_filename  = "cuc_ieschool.flv";//Input file URL
	const char *out_filename_v = "cuc_ieschool.h264";//Output file URL
	const char *out_filename_a = "cuc_ieschool.mp3";

	av_register_all();
	//Input
	if((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
		printf("==zhongjihao======Could not open input file.\n");
		return -1;
	}
	if((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		printf("===zhongjihao=====Failed to retrieve input stream information\n");
		return -1;
	}

	printf("=======zhongjihao=====ifmt_ctx->nb_streams: %d\n",ifmt_ctx->nb_streams);
	for(i=0; i<ifmt_ctx->nb_streams; i++) {
		if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			videoindex = i;
		}else if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			audioindex = i;
		}
	}
	//Dump Format------------------
	printf("\nInput Video=======zhongjihao====================\n");
	av_dump_format(ifmt_ctx, 0, in_filename, 0);
	printf("\n=============zhongjihao=========================\n");

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

			printf("==1===zhongjihao======Write Video Packet. size:%d\tpts:%lld,  data[0]: %d, %d ,%d ,%d, sps=%x\n",pkt.size,pkt.pts,pkt.data[0],pkt.data[1],pkt.data[2],pkt.data[3],pkt.data[4]);
			av_bitstream_filter_filter(h264bsfc, ifmt_ctx->streams[videoindex]->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
			printf("==2===zhongjihao======Write Video Packet. size:%d\tpts:%lld,  data[0]: %d, %d ,%d ,%d, sps=%x\n",pkt.size,pkt.pts,pkt.data[0],pkt.data[1],pkt.data[2],pkt.data[3],pkt.data[4]);
			fwrite(pkt.data,1,pkt.size,fp_video);
		}else if(pkt.stream_index == audioindex){
			/*
			AAC in some container format (FLV, MP4, MKV etc.) need to add 7 Bytes
			ADTS Header in front of AVPacket data manually.
			Other Audio Codec (MP3...) works well.
			*/
			printf("====zhongjihao======Write Audio Packet. size:%d\tpts:%lld\n",pkt.size,pkt.pts);
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
		printf("======zhongjihao=====Error occurred.\n");
		return -1;
	}
	return 0;
}


