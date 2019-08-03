/*************************************************************************
    > File Name: simplest_ffmpeg_streamer.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2018年03月13日 星期二 11时19分33秒
 ************************************************************************/

#include <stdio.h>

//#define __STDC_CONSTANT_MACROS

#ifdef __cplusplus
extern "C"
{
#endif

//#include <libavutil/log.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>

#ifdef __cplusplus
};
#endif


/*
* This example stream local media files to streaming media 
 * server (Use RTMP as example). 
 * It's the simplest FFmpeg streamer.

*/

int main(int argc, char* argv[])
{
	AVOutputFormat* ofmt = NULL;
    //Input AVFormatContext and Output AVFormatContext
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	AVPacket pkt;
	const char *in_filename, *out_filename;
	int ret, i;
	int videoindex = -1;
	int frame_index = 0;
	int64_t start_time = 0;

	in_filename  = "cuc_ieschool.flv";//输入URL (Input file URL)
	out_filename = "rtmp://localhost:1935/liveApp/room";//输出 URL（Output URL）[RTMP]

    av_log_set_level(AV_LOG_DEBUG);
	av_register_all();
	//Network
	avformat_network_init();
	//Input
	if((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) 
	{
		printf("Could not open input file.\n");
		goto end;
	}

	if((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) 
	{
		printf("Failed to retrieve input stream information\n");
		goto end;
	}

	for(i=0; i<ifmt_ctx->nb_streams; i++)
	{
		if(ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}

	av_log(NULL,AV_LOG_DEBUG,"input stream number: %d, videoindex: %d\n",ifmt_ctx->nb_streams,videoindex);
	av_dump_format(ifmt_ctx, 0, in_filename, 0);
	
	//(Output)
	avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", out_filename); //RTMP
    
	if(!ofmt_ctx) 
	{
		av_log(NULL,AV_LOG_ERROR, "Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;
	
	for(i = 0; i < ifmt_ctx->nb_streams; i++)
	{
	   //(Create output AVStream according to input AVStream)
	   AVStream *in_stream = ifmt_ctx->streams[i];
	   AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
	   if(!out_stream) 
	   {
		   av_log(NULL,AV_LOG_ERROR, "Failed allocating output stream\n");
		   ret = AVERROR_UNKNOWN;
		   goto end;
	   }
	   //(Copy the settings of AVCodecContext)
	 //  AVCodecParameters *par = avcodec_parameters_alloc();
	  // avcodec_parameters_from_context(par,in_stream->codec);
	  // ret = avcodec_parameters_to_context(out_stream->codec,par);
	   //avcodec_parameters_free(&par);
       ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
	   if(ret < 0)
	   {
		   av_log(NULL,AV_LOG_ERROR, "Failed to copy context from input to output stream codec context\n");
		   goto end;
	   }
	//   out_stream->codec->codec_tag = 0;
	   out_stream->codecpar->codec_tag = 0;
	   av_log(NULL,AV_LOG_DEBUG,"out_stream->codec->flags: %d  ofmt_ctx->oformat->flags: %x\n",out_stream->codec->flags,ofmt_ctx->oformat->flags);
	   if(ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		   out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
    
	//Dump Format------------------
	av_dump_format(ofmt_ctx, 0, out_filename, 1);
	av_log(NULL,AV_LOG_DEBUG,"name: %s, long_name: %s, mimetype: %s, flags: %d\n",ofmt->name,ofmt->long_name,ofmt->mime_type,ofmt->flags);

	//(Open output URL)
	if(!(ofmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
		if(ret < 0)
		{
			av_log(NULL,AV_LOG_ERROR, "Could not open output URL '%s'\n", out_filename);
			goto end;
		}
	}
	//(Write file header)
	ret = avformat_write_header(ofmt_ctx, NULL);
	if(ret < 0)
	{
		av_log(NULL,AV_LOG_ERROR,"Error occurred when opening output URL\n");
		goto end;
	}

	start_time = av_gettime();
	av_log(NULL,AV_LOG_DEBUG,"start_time: %d\n",start_time);	
	while (1)
	{
		AVStream *in_stream, *out_stream;
		//(Get an AVPacket)
		ret = av_read_frame(ifmt_ctx, &pkt);
		if(ret < 0)
			break;
		//FIX：No PTS (Example: Raw H.264)
		//Simple Write PTS
		if(pkt.pts == AV_NOPTS_VALUE)
		{
			//Write PTS
			AVRational time_base1 = ifmt_ctx->streams[videoindex]->time_base;
			//Duration between 2 frames (us)
			int64_t calc_duration = (double)AV_TIME_BASE/av_q2d(ifmt_ctx->streams[videoindex]->r_frame_rate);
			//Parameters
			pkt.pts = (double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
			pkt.dts = pkt.pts;
			pkt.duration = (double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
		}
		//Important:Delay
		if(pkt.stream_index == videoindex)
		{
			AVRational time_base = ifmt_ctx->streams[videoindex]->time_base;
			AVRational time_base_q = {1,AV_TIME_BASE};
			int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
			int64_t now_time = av_gettime() - start_time;
			if(pts_time > now_time)
				av_usleep(pts_time - now_time);
		}

        in_stream = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];
		/* copy packet */
		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		//Print to Screen
		if(pkt.stream_index == videoindex)
		{
			av_log(NULL,AV_LOG_DEBUG,"Send %8d video frames to output URL   videoindex: %d\n",frame_index,videoindex);
			frame_index++;
		}
	//	ret = av_write_frame(ofmt_ctx, &pkt);
		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);

		if(ret < 0)
		{
			av_log(NULL,AV_LOG_ERROR, "Error muxing packet\n");
			av_free_packet(&pkt);
			break;
		}
		
		av_free_packet(&pkt);
	}

	//(Write file trailer)
	ret = av_write_trailer(ofmt_ctx);
	av_log(NULL,AV_LOG_DEBUG, "Write file trailer ret : %d\n",ret);

	av_dump_format(ofmt_ctx, 0, out_filename, 1);
	//avformat_flush(ofmt_ctx);	
	
end:
	avformat_close_input(&ifmt_ctx);
	/* close output */

	av_log(NULL,AV_LOG_DEBUG, "ofmt->flags: %d\n",ofmt->flags);	
	if(ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	if(ret < 0 && ret != AVERROR_EOF)
	{
		av_log(NULL,AV_LOG_ERROR,"Error occurred.\n");
		return -1;
	}

	return 0;
}

