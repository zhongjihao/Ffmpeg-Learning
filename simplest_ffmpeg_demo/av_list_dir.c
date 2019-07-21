#include <libavutil/log.h>
#include <libavformat/avformat.h>

int main(int charc,char** argv)
{
    int ret;

    av_log_set_level(AV_LOG_INFO);

    AVIODirContext* ctx = NULL;
    AVIODirEntry* entry = NULL;
    ret = avio_open_dir(&ctx,"./",NULL);
    if(ret < 0){
        av_log(NULL,AV_LOG_ERROR,"Cannot open dir: %s\n",av_err2str(ret));
        return -1;
    }
    
    while (1)
    {
        ret = avio_read_dir(ctx,&entry);
        if(ret < 0){
            av_log(NULL,AV_LOG_ERROR,"Cannot read dir: %s\n",av_err2str(ret));
            goto __failed;
        }
        if(!entry){ //目录末尾
            break;
        }
        av_log(NULL,AV_LOG_INFO,"%12"PRId64" %s\n",entry->size,entry->name);
        avio_free_directory_entry(&entry);
    }

__failed:
    avio_close_dir(&ctx);
    
    return 0;
}