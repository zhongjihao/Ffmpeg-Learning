/*************************************************************************
    > File Name: simplest_ffmpeg_muxer.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
		A video stream file encoding H.264 and an audio stream file encoding MP3 or aac
		are synthesized into a file encapsulated in MP4 format.
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

/*
FIX:AAC in some container format (FLV, MP4, MKV etc.) need 
"aac_adtstoasc" bitstream filter (BSF)
*/
//'1': Use AAC Bitstream Filter 
#define USE_AACBSF 0



int main(int argc, char* argv[])
{
	AVOutputFormat *ofmt = NULL;
	//Input AVFormatContext and Output AVFormatContext
	AVFormatContext *ifmt_ctx_v = NULL, *ifmt_ctx_a = NULL,*ofmt_ctx = NULL;
	AVPacket pkt;
	int ret, i;
	int videoindex_v = -1,videoindex_out = -1;
	int audioindex_a = -1,audioindex_out = -1;
	int frame_index = 0;
	int64_t cur_pts_v = 0,cur_pts_a = 0;
	av_log_set_level(AV_LOG_DEBUG);

	//read two params from console
	if(argc < 4){
		av_log(NULL,AV_LOG_ERROR,"the count of params should be more than four\n");
		return -1;
	}

	//Input file URL
	const char *in_filename_v  = argv[1];//Input video file URL
	const char *in_filename_a = argv[2];//Input audio file URL
	const char *out_filename = argv[3];//Output file URL

	av_register_all();

	//FIX
#if USE_H264BSF
	AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb"); 
#endif
#if USE_AACBSF
	AVBitStreamFilterContext* aacbsfc =  av_bitstream_filter_init("aac_adtstoasc"); 
#endif

	//Input
	if ((ret = avformat_open_input(&ifmt_ctx_v, in_filename_v, 0, 0)) < 0) {
		av_log(NULL,AV_LOG_ERROR,"Can't open input video file: %s\n",in_filename_v);
		goto end;
	}

	if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {
		av_log(NULL,AV_LOG_ERROR,"Failed to retrieve input video stream information\n");
		goto end;
	}

	if ((ret = avformat_open_input(&ifmt_ctx_a, in_filename_a, 0, 0)) < 0) {
		av_log(NULL,AV_LOG_ERROR,"Can't open input audio file: %s\n",in_filename_a);
		goto end;
	}

	if ((ret = avformat_find_stream_info(ifmt_ctx_a, 0)) < 0) {
		av_log(NULL,AV_LOG_ERROR,"Failed to retrieve input audio stream information\n");
		goto end;
	}

	av_log(NULL,AV_LOG_INFO,"===========Input Information==========\n");
	av_dump_format(ifmt_ctx_v, 0, in_filename_v, 0);
	av_dump_format(ifmt_ctx_a, 0, in_filename_a, 0);
	av_log(NULL,AV_LOG_INFO,"======================\n");

	//Output
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
	if (!ofmt_ctx) {
		av_log(NULL,AV_LOG_ERROR,"Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;

	av_log(NULL,AV_LOG_INFO,"input video stream num: %d\n",ifmt_ctx_v->nb_streams);
	for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
		//Create output AVStream according to input AVStream
		if(ifmt_ctx_v->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			AVStream *in_stream = ifmt_ctx_v->streams[i];
			AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
			videoindex_v = i;
			if (!out_stream) {
				av_log(NULL,AV_LOG_ERROR,"Failed allocating output stream\n");
				ret = AVERROR_UNKNOWN;
				goto end;
			}
			videoindex_out = out_stream->index;
			//Copy the settings of AVCodecContext
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
				av_log(NULL,AV_LOG_ERROR,"Failed to copy context from video input to output stream codec context\n");
				goto end;
			}
			out_stream->codec->codec_tag = 0;
			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
			break;
		}
	}
	av_log(NULL,AV_LOG_INFO,"input video stream num: %d, videoindex_v: %d, videoindex_out: %d\n",ifmt_ctx_v->nb_streams,videoindex_v,videoindex_out);

	av_log(NULL,AV_LOG_INFO,"input audio stream num: %d\n",ifmt_ctx_a->nb_streams);
	for (i = 0; i < ifmt_ctx_a->nb_streams; i++) {
		//Create output AVStream according to input AVStream
		if(ifmt_ctx_a->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			AVStream *in_stream = ifmt_ctx_a->streams[i];
			AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
			audioindex_a = i;
			if (!out_stream) {
				av_log(NULL,AV_LOG_ERROR,"Failed allocating output stream\n");
				ret = AVERROR_UNKNOWN;
				goto end;
			}
			audioindex_out = out_stream->index;
			//Copy the settings of AVCodecContext
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
				av_log(NULL,AV_LOG_ERROR,"Failed to copy context from audio input to output stream codec context\n");
				goto end;
			}
			out_stream->codec->codec_tag = 0;
			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

			break;
		}
	}
	av_log(NULL,AV_LOG_INFO,"input audio stream num: %d, audioindex_a: %d, audioindex_out: %d\n",ifmt_ctx_a->nb_streams,audioindex_a,audioindex_out);
	
	av_log(NULL,AV_LOG_INFO,"===========Output Information==========\n");
	av_dump_format(ofmt_ctx, 0, out_filename, 1);
	av_log(NULL,AV_LOG_INFO,"======================\n");

	//Open output file
	if (!(ofmt->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
			av_log(NULL,AV_LOG_ERROR,"Could not open output file '%s'\n",out_filename);
			goto end;
		}
	}

	//Write file header
	if (avformat_write_header(ofmt_ctx, NULL) < 0) {
		av_log(NULL,AV_LOG_ERROR,"Error occurred when opening output file\n");
		goto end;
	}


	while (1) {
		AVFormatContext *ifmt_ctx;
		int stream_index = 0;
		AVStream *in_stream, *out_stream;

		//Get an AVPacket
		//Compare timestamps to decide whether to write video or audio
		if(av_compare_ts(cur_pts_v,ifmt_ctx_v->streams[videoindex_v]->time_base,cur_pts_a,ifmt_ctx_a->streams[audioindex_a]->time_base) <= 0){
			ifmt_ctx = ifmt_ctx_v;
			stream_index = videoindex_out;

			if(av_read_frame(ifmt_ctx, &pkt) >= 0){
				do{
					in_stream  = ifmt_ctx->streams[pkt.stream_index];
					out_stream = ofmt_ctx->streams[stream_index];

					if(pkt.stream_index == videoindex_v){
						//FIX：No PTS (Example: Raw H.264)
						//Simple Write PTS
						if(pkt.pts == AV_NOPTS_VALUE){
							//Write PTS
							AVRational time_base1 = in_stream->time_base;
							//Duration between 2 frames (us)
							int64_t calc_duration = (double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
							//Parameters
							pkt.pts = (double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts = pkt.pts;
							pkt.duration = (double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							frame_index++;
						}

						cur_pts_v = pkt.pts;
						break;
					}
				}while(av_read_frame(ifmt_ctx, &pkt) >= 0);
			}else{
				break;
			}
		}else{
			ifmt_ctx = ifmt_ctx_a;
			stream_index = audioindex_out;
			if(av_read_frame(ifmt_ctx, &pkt) >= 0){
				do{
					in_stream  = ifmt_ctx->streams[pkt.stream_index];
					out_stream = ofmt_ctx->streams[stream_index];

					if(pkt.stream_index == audioindex_a){

						//FIX：No PTS
						//Simple Write PTS
						if(pkt.pts == AV_NOPTS_VALUE){
							//Write PTS
							AVRational time_base1 = in_stream->time_base;
							//Duration between 2 frames (us)
							int64_t calc_duration = (double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
							//Parameters
							pkt.pts = (double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts = pkt.pts;
							pkt.duration = (double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							frame_index++;
						}
						cur_pts_a = pkt.pts;

						break;
					}
				}while(av_read_frame(ifmt_ctx, &pkt) >= 0);
			}else{
				break;
			}

		}

		//FIX:Bitstream Filter
#if USE_H264BSF
		av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
#if USE_AACBSF
		av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif


		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = stream_index;

		av_log(NULL,AV_LOG_DEBUG,"Write 1 Packet. size:%5d\tpts:%lld,   pkt.stream_index: %d\n",pkt.size,pkt.pts,pkt.stream_index);
		//Write
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			av_log(NULL,AV_LOG_ERROR,"Error muxing packet\n");
			break;
		}
		av_free_packet(&pkt);

	}
	//Write file trailer
	av_write_trailer(ofmt_ctx);

#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);
#endif
#if USE_AACBSF
	av_bitstream_filter_close(aacbsfc);
#endif

end:
	avformat_close_input(&ifmt_ctx_v);
	avformat_close_input(&ifmt_ctx_a);
	/* close output */
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	if (ret < 0 && ret != AVERROR_EOF) {
		av_log(NULL,AV_LOG_ERROR,"Error occurred\n");
		return -1;
	}

	return 0;
}


