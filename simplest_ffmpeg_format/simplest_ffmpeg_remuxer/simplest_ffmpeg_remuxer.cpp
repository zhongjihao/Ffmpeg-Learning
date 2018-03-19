/*************************************************************************
    > File Name: simplest_ffmpeg_remuxer.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2018年03月16日 星期五 11时04分25秒
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



int main(int argc, char* argv[])
{
	AVOutputFormat *ofmt = NULL;
	//Input AVFormatContext and Output AVFormatContext
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	AVPacket pkt;
	const char *in_filename, *out_filename;
	int ret, i;
	int frame_index = 0;
	in_filename  = "cuc_ieschool1.flv";//Input file URL
	out_filename = "cuc_ieschool1.mp4";//Output file URL

	av_register_all();
	//Input
	if((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
		printf("===zhongjihao===Could not open input file.");
		goto end;
	}

	if((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		printf("==zhongjihao====Failed to retrieve input stream information");
		goto end;
	}

	av_dump_format(ifmt_ctx, 0, in_filename, 0);
	//Output
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
	if(!ofmt_ctx) {
		printf("===zhongjihao===Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;

	for(i = 0; i < ifmt_ctx->nb_streams; i++) {
		//Create output AVStream according to input AVStream
		AVStream *in_stream = ifmt_ctx->streams[i];
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
		if(!out_stream) {
			printf("====zhongjihao====Failed allocating output stream\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		//Copy the settings of AVCodecContext
		if(avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
			printf("====zhongjihao=====Failed to copy context from input to output stream codec context\n");
			goto end;
		}
		out_stream->codec->codec_tag = 0;
		if(ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
			printf("====zhongjihao====oformat->flags : %d\n",ofmt_ctx->oformat->flags);
			out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
	}
	printf("====zhongjihao====nb_streams : %d\n",ifmt_ctx->nb_streams);
	//Output information------------------
	av_dump_format(ofmt_ctx, 0, out_filename, 1);

	//Open output file
	if(!(ofmt->flags & AVFMT_NOFILE)) {
	    printf("====zhongjihao====ofmt->flags : %d\n",ofmt->flags);
		ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
		if(ret < 0) {
			printf("======zhongjihao======Could not open output file '%s'", out_filename);
			goto end;
		}
	}
	//Write file header
	if(avformat_write_header(ofmt_ctx, NULL) < 0) {
		printf("====zhongjihao====Error occurred when opening output file\n");
		goto end;
	}

	while (1) {
		AVStream *in_stream, *out_stream;
		//Get an AVPacket
		ret = av_read_frame(ifmt_ctx, &pkt);
		if(ret < 0)
			break;
		in_stream  = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];

		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		//Write
		if(av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			printf("====zhongjihao=====Error muxing packet\n");
			break;
		}
		printf("====zhongjihao====Write %8d frames to output file,   pkt.stream_index: %d\n",frame_index,pkt.stream_index);
		av_free_packet(&pkt);
		frame_index++;
	}
	//Write file trailer
	av_write_trailer(ofmt_ctx);

	//Output information------------------
	av_dump_format(ofmt_ctx, 0, out_filename, 1);

end:
	avformat_close_input(&ifmt_ctx);
	/* close output */
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	
	printf("====zhongjihao=====remuxe end\n");
	return 0;
}

