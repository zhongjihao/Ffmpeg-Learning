/*************************************************************************
    > File Name: simplest_ffmpeg_demuxer.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 

 * This software split a media file (in Container such as 
 * MKV, FLV, AVI...) to video and audio bitstream.
 * In this example, it demux a MPEG2TS file to H.264 bitstream
 * and AAC bitstream.
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

/*
FIX: H.264 in some container format (FLV, MP4, MKV etc.) need 
"h264_mp4toannexb" bitstream filter (BSF)
  *Add SPS,PPS in front of IDR frame
  *Add start code ("0,0,0,1") in front of NALU
H.264 in some container (MPEG2TS) don't need this BSF.
*/
//'1': Use H.264 Bitstream Filter 
#define USE_H264BSF 0

int main(int argc, char* argv[])
{
	AVOutputFormat *ofmt_a = NULL,*ofmt_v = NULL;
	//（Input AVFormatContext and Output AVFormatContext）
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx_a = NULL, *ofmt_ctx_v = NULL;
	AVPacket pkt;
	int ret, i;
	int videoindex = -1,audioindex = -1;
	int frame_index = 0;

	const char *in_filename  = NULL;//Input file URL
	const char *out_filename_v = NULL;//Output file URL
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

#if USE_H264BSF
	AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb"); 
#endif

	//Input
	if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
		av_log(NULL,AV_LOG_ERROR,"Could not open input file %s.\n",in_filename);
		goto end;
	}

	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		av_log(NULL,AV_LOG_ERROR,"Failed to retrieve input stream information\n");
		goto end;
	}

	//Output
	avformat_alloc_output_context2(&ofmt_ctx_v, NULL, NULL, out_filename_v);
	if (!ofmt_ctx_v) {
		av_log(NULL,AV_LOG_ERROR,"Could not create video output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt_v = ofmt_ctx_v->oformat;

	avformat_alloc_output_context2(&ofmt_ctx_a, NULL, NULL, out_filename_a);
	if (!ofmt_ctx_a) {
		av_log(NULL,AV_LOG_ERROR,"Could not create audio output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt_a = ofmt_ctx_a->oformat;

	av_log(NULL,AV_LOG_DEBUG,"ifmt_ctx->nb_streams: %d\n",ifmt_ctx->nb_streams);
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
			//Create output AVStream according to input AVStream
			AVFormatContext *ofmt_ctx;
			AVStream *in_stream = ifmt_ctx->streams[i];
			AVStream *out_stream = NULL;
			
			if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
				videoindex = i;
				out_stream = avformat_new_stream(ofmt_ctx_v, in_stream->codec->codec);
				ofmt_ctx = ofmt_ctx_v;
			}else if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
				audioindex = i;
				out_stream = avformat_new_stream(ofmt_ctx_a, in_stream->codec->codec);
				ofmt_ctx = ofmt_ctx_a;
			}else{
				break;
			}
			
			if (!out_stream) {
				av_log(NULL,AV_LOG_ERROR,"Failed allocating output stream\n");
				ret = AVERROR_UNKNOWN;
				goto end;
			}
			//Copy the settings of AVCodecContext
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
				av_log(NULL,AV_LOG_ERROR,"Failed to copy context from input to output stream codec context\n");
				goto end;
			}
			out_stream->codec->codec_tag = 0;

			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	av_log(NULL,AV_LOG_DEBUG,"ifmt_ctx->nb_streams: %d, videoindex: %d, audioindex: %d\n",ifmt_ctx->nb_streams,videoindex,audioindex);
	//Dump Format------------------
	av_log(NULL,AV_LOG_DEBUG,"\n===========Input Video=============\n");
	av_dump_format(ifmt_ctx, 0, in_filename, 0);
	av_log(NULL,AV_LOG_DEBUG,"\n===========Output Video============\n");
	av_dump_format(ofmt_ctx_v, 0, out_filename_v, 1);
	av_log(NULL,AV_LOG_DEBUG,"\n========Output Audio============\n");
	av_dump_format(ofmt_ctx_a, 0, out_filename_a, 1);
	av_log(NULL,AV_LOG_DEBUG,"\n===============================\n");

	//Open output file
	if (!(ofmt_v->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx_v->pb, out_filename_v, AVIO_FLAG_WRITE) < 0) {
			av_log(NULL,AV_LOG_ERROR,"Could not open video output file '%s'\n", out_filename_v);
			goto end;
		}
	}

	if (!(ofmt_a->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx_a->pb, out_filename_a, AVIO_FLAG_WRITE) < 0) {
			av_log(NULL,AV_LOG_ERROR,"Could not open audio output file '%s'\n", out_filename_a);
			goto end;
		}
	}

	//Write file header
	if (avformat_write_header(ofmt_ctx_v, NULL) < 0) {
		av_log(NULL,AV_LOG_ERROR,"Error occurred when opening video output file\n");
		goto end;
	}
	if (avformat_write_header(ofmt_ctx_a, NULL) < 0) {
		av_log(NULL,AV_LOG_ERROR,"Error occurred when opening audio output file\n");
		goto end;
	}
	
	while (1) {
		AVFormatContext *ofmt_ctx;
		AVStream *in_stream, *out_stream;
		//Get an AVPacket
		if (av_read_frame(ifmt_ctx, &pkt) < 0)
			break;
		in_stream  = ifmt_ctx->streams[pkt.stream_index];

		
		if(pkt.stream_index == videoindex){
			out_stream = ofmt_ctx_v->streams[0];
			ofmt_ctx = ofmt_ctx_v;
			av_log(NULL,AV_LOG_DEBUG,"==1===Write Video Packet. size:%d\tpts:%lld, nb_streams: %d, data[0]: %x, %x, %x, %x ,%x, sps: %x ,%x ,%x, %x,%x\n",pkt.size,pkt.pts,
					ofmt_ctx->nb_streams,pkt.data[0],pkt.data[1],pkt.data[2],pkt.data[3],pkt.data[4],out_stream->codec->extradata[0],out_stream->codec->extradata[1],
					out_stream->codec->extradata[2],out_stream->codec->extradata[3],out_stream->codec->extradata[4]);
#if USE_H264BSF
			av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
			av_log(NULL,AV_LOG_DEBUG,"==2===Write Video Packet. size:%d\tpts:%lld, nb_streams: %d, data[0]: %x, %x, %x, %x ,%x, sps: %x ,%x ,%x, %x,%x\n",pkt.size,pkt.pts,
					ofmt_ctx->nb_streams,pkt.data[0],pkt.data[1],pkt.data[2],pkt.data[3],pkt.data[4],out_stream->codec->extradata[0],out_stream->codec->extradata[1],
					out_stream->codec->extradata[2],out_stream->codec->extradata[3],out_stream->codec->extradata[4]);
#endif
		}else if(pkt.stream_index == audioindex){
			out_stream = ofmt_ctx_a->streams[0];
			ofmt_ctx = ofmt_ctx_a;
			av_log(NULL,AV_LOG_DEBUG,"Write Audio Packet. size:%d\tpts:%lld, nb_streams: %d, data[0]: %x, %x, %x, %x, %x ,%x ,%x\n",pkt.size,pkt.pts,
					ofmt_ctx->nb_streams,pkt.data[0],pkt.data[1],pkt.data[2],pkt.data[3],pkt.data[4],pkt.data[5],pkt.data[6]);
		}else{
			continue;
		}

		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = 0;
		//Write
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			av_log(NULL,AV_LOG_ERROR,"Error muxing packet\n");
			break;
		}
		//av_log(NULL,AV_LOG_DEBUG,"Write %8d frames to output file\n",frame_index);
		av_free_packet(&pkt);
		frame_index++;
	}

#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);  
#endif

	//Write file trailer
	av_write_trailer(ofmt_ctx_a);
	av_write_trailer(ofmt_ctx_v);
end:
	avformat_close_input(&ifmt_ctx);
	/* close output */
	if (ofmt_ctx_a && !(ofmt_a->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx_a->pb);

	if (ofmt_ctx_v && !(ofmt_v->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx_v->pb);

	avformat_free_context(ofmt_ctx_a);
	avformat_free_context(ofmt_ctx_v);


	if (ret < 0 && ret != AVERROR_EOF) {
		av_log(NULL,AV_LOG_ERROR,"Error occurred.\n");
		return -1;
	}

	return 0;
}


