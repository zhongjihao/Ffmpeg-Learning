#include <libavformat/avformat.h>
#include <libavutil/log.h>

int main(int argc,char** argv)
{
    int ret;
    ret = avpriv_io_move("111.txt","zhongjihao.txt");
    if(ret < 0){
        av_log(NULL,AV_LOG_ERROR,"Failed to rename\n");
        return -1;
    }
    av_log(NULL,AV_LOG_INFO,"Success to rename\n");

    ret = avpriv_io_delete("./ffmpeg_log");
    if(ret <0){
        av_log(NULL,AV_LOG_ERROR,"Failed to delete file ffmpeg_log\n");
        return -1;
    }
    av_log(NULL,AV_LOG_INFO,"Success to delete ffmpeg_log\n");
    return 0;
}