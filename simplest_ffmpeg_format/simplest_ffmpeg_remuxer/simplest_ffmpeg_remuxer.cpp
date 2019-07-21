/*************************************************************************
    > File Name: simplest_ffmpeg_remuxer.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2018年03月16日 星期五 11时04分25秒
	本程序实现了视频封装格式之间的转换。
	需要注意的是本程序并不改变视音频的编码格式。
 ************************************************************************/


#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavutil/log.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#ifdef __cplusplus
};
#endif



int main(int argc, char* argv[])
{
	char errors[1024];
	AVOutputFormat *ofmt = NULL;
	AVBitStreamFilterContext *vbsf = NULL;
	//Input AVFormatContext and Output AVFormatContext
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	AVPacket pkt;
	const char *in_filename, *out_filename;
	int ret, i;
	int frame_index = 0;
	
	av_log_set_level(AV_LOG_DEBUG);

    //read two params from console
	if(argc < 3){
		av_log(NULL,AV_LOG_ERROR,"the count of params should be more than three\n");
		return -1;
	}

	in_filename  = argv[1];//Input file URL
	out_filename = argv[2];//Output file URL

	av_register_all();

	//打开输入文件，初始化输入视频码流的AVFormatContex
	if((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
		av_strerror(ret,errors,1024);
		av_log(NULL,AV_LOG_ERROR,"Can't open input file: %s,%d(%s)\n",in_filename,ret,errors);
		goto end;
	}

	if((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		av_strerror(ret,errors,1024);
		av_log(NULL,AV_LOG_ERROR,"Failed to retrieve input stream information: %d(%s)\n",ret,errors);
		goto end;
	}

	av_dump_format(ifmt_ctx, 0, in_filename, 0);

	vbsf = av_bitstream_filter_init("h264_mp4toannexb");
	if(vbsf == NULL){
		av_log(NULL,AV_LOG_ERROR,"bitstream filter is null\n");
		goto end;
	}
    av_register_bitstream_filter(const_cast<AVBitStreamFilter*>(vbsf->filter));

	//初始化输出视频码流的AVFormatContext
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
	if(!ofmt_ctx) {
		av_log(NULL,AV_LOG_ERROR,"Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;

	for(i = 0; i < ifmt_ctx->nb_streams; i++) {
		//根据输入流创建输出流(Create output AVStream according to input AVStream)
		AVStream *in_stream = ifmt_ctx->streams[i];
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
		if(!out_stream) {
            av_log(NULL,AV_LOG_ERROR,"Failed allocating output stream\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		//复制AVCodecContext的设置(Copy the settings of AVCodecContext)
		ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
		if(ret < 0){
			av_strerror(ret,errors,1024);
            av_log(NULL,AV_LOG_ERROR,"Failed to copy context from input to output stream codec context: %d(%s)\n",ret,errors);
			goto end;
		}
		out_stream->codec->codec_tag = 0;
		if(ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
            av_log(NULL,AV_LOG_DEBUG,"oformat->flags: %d\n",ofmt_ctx->oformat->flags);
			out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
	}

    av_log(NULL,AV_LOG_DEBUG,"input file nb_streams: %d\n",ifmt_ctx->nb_streams);
	//Output information------------------
	av_dump_format(ofmt_ctx, 0, out_filename, 1);

	//打开输出文件(Open output file)
	if(!(ofmt->flags & AVFMT_NOFILE)) {
        av_log(NULL,AV_LOG_DEBUG,"ofmt->flags: %d\n",ofmt->flags);
		ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
		if(ret < 0) {
            av_log(NULL,AV_LOG_ERROR,"Could not open output file :%s\n",out_filename);
			goto end;
		}
	}
	//Write file header
	ret = avformat_write_header(ofmt_ctx, NULL);
	if(ret < 0){
		av_strerror(ret,errors,1024);
        av_log(NULL,AV_LOG_ERROR,"Error occurred when opening output file: %d(%s)\n",ret,errors);
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

		//转换PTS/DTS(Convert PTS/DTS)
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;

		if(pkt.stream_index == 0){
			 AVPacket fpkt = pkt; 
			 int a = av_bitstream_filter_filter(vbsf,out_stream->codec,NULL,
					                            &fpkt.data,&fpkt.size,pkt.data,pkt.size ,
												pkt.flags & AV_PKT_FLAG_KEY); 
			 pkt.data = fpkt.data;
			 pkt.size = fpkt.size; 
		}

		//写入(Write)
		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
		if(ret < 0){
			av_strerror(ret,errors,1024);
            av_log(NULL,AV_LOG_ERROR,"Error muxing packet: %d(%s)\n",ret,errors);
			break;
		}
	
		av_log(NULL,AV_LOG_DEBUG,"Write %8d frames to output file,pkt.stream_index: %d\n",frame_index,pkt.stream_index);
		av_free_packet(&pkt);
		frame_index++;
	}
	//写文件尾(Write file trailer)
	av_write_trailer(ofmt_ctx);

	//Output information------------------
	av_dump_format(ofmt_ctx, 0, out_filename, 1);

end:
	avformat_close_input(&ifmt_ctx);
	/* close output */
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);

	if (ret < 0 && ret != AVERROR_EOF) {
		printf( "Error occurred.\n");
		av_log(NULL,AV_LOG_ERROR,"Error occurred\n");
		return -1;
	}
	av_log(NULL,AV_LOG_DEBUG,"remutex ok\n");
	return 0;
}

