/**
 * 分离出aac编码的音频
*/

#include <stdio.h>
#include <libavutil/log.h>
#include <libavformat/avformat.h>

#define ADTS_HEADER_LEN  7;

void adts_header(char *szAdtsHeader, int dataLen){

    int audio_object_type = 2; // AAC LC
    int sampling_frequency_index = 4; //采样率的下标 0x4代表44100,0x8代表16000,0x3代表48000,0x2代表64000
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

int main(int argc,char* argv[])
{
    int ret;
    int len;
    int audio_index = -1;
    char* src = NULL;
    char* dst = NULL;
    AVFormatContext* fmt_ctx = NULL;
    AVPacket pkt;
    av_log_set_level(AV_LOG_INFO);

    av_register_all();

    //1 read two params from console
    if(argc < 3){
        av_log(NULL,AV_LOG_ERROR,"the count of params should be more than three\n");
        return -1;
    }

    src = argv[1];
    dst = argv[2];
    if(!src || !dst){
        av_log(NULL,AV_LOG_ERROR,"src or dst is null\n");
        return -1;
    }

    ret = avformat_open_input(&fmt_ctx,src,NULL,NULL);
    if(ret <0){
        av_log(NULL,AV_LOG_ERROR,"Can't open file: %s\n",av_err2str(ret));
        return -1;
    }

    FILE* dst_fd = fopen(dst,"wb");
    if(!dst_fd){
        av_log(NULL,AV_LOG_ERROR,"Can't open out file\n");
        avformat_close_input(&fmt_ctx);
        return -1;
    }
    av_dump_format(fmt_ctx,0,src,0);

    //2 get stream
    ret = av_find_best_stream(fmt_ctx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
    if(ret <0){
        av_log(NULL,AV_LOG_ERROR,"Can't find the best stream\n");
        avformat_close_input(&fmt_ctx);
        fclose(dst_fd);
        return -1;
    }
    audio_index = ret;

    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
   
    while(av_read_frame(fmt_ctx,&pkt) >= 0){
        if(pkt.stream_index == audio_index){
            char adts_header_buf[7];
            adts_header(adts_header_buf, pkt.size);
            fwrite(adts_header_buf, 1, 7, dst_fd);

           //3 write audio data to aac file
           len = fwrite(pkt.data,1,pkt.size,dst_fd);
           if(len != pkt.size){
               av_log(NULL,AV_LOG_WARNING,"warning,length of data is not equal size of pkt!\n");
           }
        }
        av_packet_unref(&pkt);
    }

    if(dst_fd){
        fclose(dst_fd);
    }

    avformat_close_input(&fmt_ctx);

    return 0;
}