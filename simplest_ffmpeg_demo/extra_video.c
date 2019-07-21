/**
 * 分离出h264视频
*/

#include <stdio.h>
#include <libavutil/log.h>
#include <libavformat/avio.h>
#include <libavformat/avformat.h>

#ifndef AV_WB32
#   define AV_WB32(p, val) do {                 \
        uint32_t d = (val);                     \
        ((uint8_t*)(p))[3] = (d);               \
        ((uint8_t*)(p))[2] = (d)>>8;            \
        ((uint8_t*)(p))[1] = (d)>>16;           \
        ((uint8_t*)(p))[0] = (d)>>24;           \
    } while(0)
#endif

#ifndef AV_RB16
#   define AV_RB16(x)                           \
    ((((const uint8_t*)(x))[0] << 8) |          \
      ((const uint8_t*)(x))[1])
#endif

static int alloc_and_copy(AVPacket *out,
                          const uint8_t *sps_pps, uint32_t sps_pps_size,
                          const uint8_t *in, uint32_t in_size)
{
    uint32_t offset         = out->size;
    uint8_t nal_header_size = offset ? 3 : 4;
    av_log(NULL,AV_LOG_INFO,"alloc_and_copy----->sps_pps: %p,offset: %d,sps_pps_size: %d,in_size: %d\n",sps_pps,offset,sps_pps_size,in_size);
    int err;

    err = av_grow_packet(out, sps_pps_size + in_size + nal_header_size);
    if (err < 0)
        return err;

    if (sps_pps)
        memcpy(out->data + offset, sps_pps, sps_pps_size);
    memcpy(out->data + sps_pps_size + nal_header_size + offset, in, in_size);
    if (!offset) {
        AV_WB32(out->data + sps_pps_size, 1);
    } else {
        (out->data + offset + sps_pps_size)[0] = 0;
        (out->data + offset + sps_pps_size)[1] = 0;
        (out->data + offset + sps_pps_size)[2] = 1;
    }

    return 0;
}

int h264_extradata_to_annexb(const uint8_t *codec_extradata, const int codec_extradata_size, AVPacket *out_extradata, int padding)
{
    uint16_t unit_size = 0;
    uint64_t total_size = 0;
    uint8_t *out = NULL;
    uint8_t unit_nb = 0;
    uint8_t sps_done = 0;
    uint8_t sps_seen = 0;
    uint8_t pps_seen = 0;
    uint8_t sps_offset = 0;
    uint8_t pps_offset = 0;
    av_log(NULL,AV_LOG_INFO,"extradata: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%d\n",codec_extradata[0],codec_extradata[1],codec_extradata[2],
                                                                          codec_extradata[3],codec_extradata[4],codec_extradata[5],
                                                                          codec_extradata[6],codec_extradata[7],codec_extradata[8],
                                                                          codec_extradata[9],codec_extradata_size);
    const uint8_t *extradata = codec_extradata + 4; //跳过前4个字节,指向ff,e1,0,17--->sps数据大小,67--->sps数据,其中ff,e1,0这头三个字节为固定值
    static const uint8_t nalu_header[4] = { 0, 0, 0, 1 };
    int length_size = (*extradata++ & 0x3) + 1;
    av_log(NULL,AV_LOG_INFO,"extradata: %x,%x\n",length_size,*extradata);
    sps_offset = pps_offset = -1;

    /* retrieve sps and pps unit(s) */
    unit_nb = *extradata++ & 0x1f; /* number of sps unit(s) */
    av_log(NULL,AV_LOG_INFO,"unit_nb: %d\n",unit_nb);
    if (!unit_nb) {
        goto pps;
    }else {
        sps_offset = 0;
        sps_seen = 1;
    }
   
    while (unit_nb--) {
        int err;

        unit_size   = AV_RB16(extradata); //获取sps/pps的Nalu大小
        av_log(NULL,AV_LOG_INFO,"unit_nb: %d,unit_size: %d\n",unit_nb,unit_size);
        total_size += unit_size + 4; //nalu大小加上4字节的起始码0x00000001
        if (total_size > INT_MAX - padding) {
            av_log(NULL, AV_LOG_ERROR,
                   "Too big extradata size, corrupted stream or invalid MP4/AVCC bitstream\n");
            av_free(out);
            return AVERROR(EINVAL);
        }
        if (extradata + 2 + unit_size > codec_extradata + codec_extradata_size) { //sps/pps的Nalu大小超过了AVCodecContext的extradata的数据大小extradata_size
            av_log(NULL, AV_LOG_ERROR, "Packet header is not contained in global extradata, "
                   "corrupted stream or invalid MP4/AVCC bitstream\n");
            av_free(out);
            return AVERROR(EINVAL);
        }
        if ((err = av_reallocp(&out, total_size + padding)) < 0)
            return err;
        memcpy(out + total_size - unit_size - 4, nalu_header, 4); //先拷贝4字节的起始码0x00000001
        av_log(NULL,AV_LOG_INFO,"sps/pps: %x,unit_size: %d\n",*(extradata + 2),unit_size);
        memcpy(out + total_size - unit_size, extradata + 2, unit_size); //拷贝sps/pps的nalu数据
        extradata += 2 + unit_size; //跳到sps/pps的nalu末尾
        av_log(NULL,AV_LOG_INFO,"pps: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",extradata[0],extradata[1],extradata[2],
                                                                          extradata[3],extradata[4],extradata[5],
                                                                          extradata[6],extradata[7],extradata[8],
                                                                          extradata[9]);
        av_log(NULL,AV_LOG_INFO,"unit_nb: %d,sps_done: %d\n",unit_nb,sps_done);                                                                
pps:
        if (!unit_nb && !sps_done++) {
            unit_nb = *extradata++; /* number of pps unit(s) 将unit_nb重置为1,进行下一次循环提取pps*/
            av_log(NULL,AV_LOG_INFO,"unit_nb: %d,pps: %x,%x,%x,%x,%x,%x,%x,%x,%x\n",unit_nb,extradata[0],extradata[1],
                                                                          extradata[2],extradata[3],extradata[4],
                                                                          extradata[5],extradata[6],extradata[7],
                                                                          extradata[8]);
            if (unit_nb) {
                pps_offset = total_size;
                pps_seen = 1;
            }
        }
    }

    if (out)
        memset(out + total_size, 0, padding);

    if (!sps_seen)
        av_log(NULL, AV_LOG_WARNING,
               "Warning: SPS NALU missing or invalid. "
               "The resulting stream may not play.\n");

    if (!pps_seen)
        av_log(NULL, AV_LOG_WARNING,
               "Warning: PPS NALU missing or invalid. "
               "The resulting stream may not play.\n");

    out_extradata->data      = out; //将包含起始码的sps和pps数据首地址保存到data中
    out_extradata->size      = total_size;

    return length_size;
}

int h264_mp4toannexb(AVFormatContext *fmt_ctx, AVPacket *in, FILE *dst_fd)
{

    AVPacket *out = NULL;
    AVPacket spspps_pkt;

    int len;
    uint8_t unit_type;
    int32_t nal_size;
    uint32_t cumul_size    = 0;
    const uint8_t *buf;
    const uint8_t *buf_end;
    int            buf_size;
    int ret = 0, i;

    out = av_packet_alloc();

    buf      = in->data;
    buf_size = in->size;
    buf_end  = in->data + in->size;

    do {
        ret= AVERROR(EINVAL);
        if (buf + 4 > buf_end)
            goto fail;

        //每个AVPacket中data的头4个字节代表nalu大小
        for (nal_size = 0, i = 0; i<4; i++)
            nal_size = (nal_size << 8) | buf[i];

        buf += 4; //跳过表示nalu大小的4个字节
        unit_type = *buf & 0x1f; //获取nalu类型

        if (nal_size > buf_end - buf || nal_size < 0)
            goto fail;

        /* prepend only to the first type 5 NAL unit of an IDR picture, if no sps/pps are already present */
        if (unit_type == 5 ) { //IDR帧需要添加sps和pps
            h264_extradata_to_annexb( fmt_ctx->streams[in->stream_index]->codec->extradata,
                                      fmt_ctx->streams[in->stream_index]->codec->extradata_size,
                                      &spspps_pkt,
                                      AV_INPUT_BUFFER_PADDING_SIZE);

            if ((ret=alloc_and_copy(out,
                               spspps_pkt.data, spspps_pkt.size,
                               buf, nal_size)) < 0)
                goto fail;
        } else {
            if ((ret=alloc_and_copy(out, NULL, 0, buf, nal_size)) < 0)
                goto fail;
        }

        //将nalu数据写入到文件
        len = fwrite( out->data, 1, out->size, dst_fd);
        if(len != out->size){
            av_log(NULL, AV_LOG_DEBUG, "warning, length of writed data isn't equal pkt.size(%d, %d)\n",
                    len,
                    out->size);
        }
        fflush(dst_fd);

next_nal:
        buf        += nal_size;
        cumul_size += nal_size + 4;
    } while (cumul_size < buf_size);

fail:
    av_packet_free(&out);

    return ret;
}

int main(int argc,char* argv[])
{
    int err_code;
    char errors[1024];
    char *src_filename = NULL;
    char *dst_filename = NULL;

    FILE *dst_fd = NULL;
    int video_stream_index = -1;

    AVFormatContext* fmt_ctx = NULL;
    AVPacket pkt;

    av_log_set_level(AV_LOG_INFO);

    //1 read two params from console
    if(argc < 3){
        av_log(NULL,AV_LOG_ERROR,"the count of params should be more than three\n");
        return -1;
    }

    src_filename = argv[1];
    dst_filename = argv[2];
    if(!src_filename || !dst_filename){
        av_log(NULL,AV_LOG_ERROR,"src or dst is null\n");
        return -1;
    }

    //register all formats and codec
    av_register_all();

    dst_fd = fopen(dst_filename,"wb");
    if(!dst_fd){
        av_log(NULL,AV_LOG_ERROR,"Can't open out file\n");
        return -1;
    }

    /*open input media file, and allocate format context*/
    err_code = avformat_open_input(&fmt_ctx,src_filename,NULL,NULL);
    if(err_code <0){
        av_strerror(err_code,errors,1024);
        av_log(NULL,AV_LOG_ERROR,"Can't open source file: %s,%d(%s)\n",src_filename,err_code,errors);
        fclose(dst_fd);
        return -1;
    }

    /*dump input information*/
    av_dump_format(fmt_ctx,0,src_filename,0);

    /*initialize packet*/
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    /*find best video stream*/
    video_stream_index = av_find_best_stream(fmt_ctx,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
    if(video_stream_index <0){
        av_log(NULL,AV_LOG_ERROR,"Can't find %s the best stream in input file %s\n",av_get_media_type_string(AVMEDIA_TYPE_VIDEO),src_filename);
        avformat_close_input(&fmt_ctx);
        fclose(dst_fd);
        return AVERROR(EINVAL);
    }

    /*read frames from media file*/
    while(av_read_frame(fmt_ctx,&pkt) >= 0){
        if(pkt.stream_index == video_stream_index){
            h264_mp4toannexb(fmt_ctx, &pkt, dst_fd);
        }
        //release pkt->data
        av_packet_unref(&pkt);
    }
    
    /*close input media file*/
    avformat_close_input(&fmt_ctx);
    if(dst_fd){
        fclose(dst_fd);
    }

    return 0;
}