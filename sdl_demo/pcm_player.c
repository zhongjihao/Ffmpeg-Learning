/*************************************************************************
    > File Name: pcm_player.c
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2019年10月25日 19时45分22秒 CST
 ************************************************************************/

#include <SDL2/SDL.h>

#define BLOCK_SIZE 4096000
static Uint8* audio_buf = NULL;
static Uint8* audio_pos = NULL;
static size_t buffer_len = 0;

//callback function for audio devcie
void read_audio_data(void* userdata ,Uint8 * stream,int len)
{   
    SDL_Log("read_audio_data-------->stream: %p, len: %d ,buffer_len: %ld\n",stream,len,buffer_len);
    if(buffer_len == 0){
        return;
    }
    //清空SDL音频数据缓冲区
    SDL_memset(stream,0,len);
    len = (len < buffer_len) ? len : buffer_len;
    SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);
    audio_pos += len;
}

int main(int argc, char* argv[])
{
    int ret = -1;
    const char* path = "NocturneNo2inEflat_44.1k_s16le.pcm";
    FILE* audio_fd = NULL;

    //initialize SDL
	if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		SDL_Log("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
    
    audio_fd = fopen(path,"r");
    if(!audio_fd){
        SDL_Log("Failed to open pcm file\n");
        goto __FAILED;
    }

    audio_buf = (Uint8*)malloc(BLOCK_SIZE);
    if(!audio_buf){
        SDL_Log("Failed to malloc memory!\n");
        goto __FAILED;
    }
    //设置音频参数
    SDL_AudioSpec spec;
    spec.freq = 44100;
    spec.channels = 2;
    spec.silence = 0;
    spec.format = AUDIO_S16SYS;
    spec.callback = read_audio_data;
    spec.userdata = NULL;
    //打开音频设备
    if(SDL_OpenAudio(&spec,NULL)){
        SDL_Log("Failed to open audio device - %s\n", SDL_GetError());
        goto __FAILED;
    }

    SDL_Log("Open Audio device ok\n");
    //启动播放 0--播放状态  1--暂停状态
    SDL_PauseAudio(0);
    do{
        buffer_len = fread(audio_buf,1,BLOCK_SIZE,audio_fd);
        audio_pos = audio_buf;
        SDL_Log("fread-------->audio_buf: %p, audio_pos: %p ,buffer_len: %ld\n",audio_buf,audio_pos,buffer_len);
        while(audio_pos < (audio_buf+buffer_len)){
            SDL_Delay(1);
        }
    }while (buffer_len != 0);
   
    SDL_Log("Close Audio device\n");
    //关闭音频设备
    SDL_CloseAudio();

    ret = 0;

__FAILED: 
    if(audio_buf){
        free(audio_buf);
    }
    if(audio_fd){
        fclose(audio_fd);
    }
    SDL_Quit();

    return ret;
}