/*************************************************************************
    > File Name: yuv_player.c
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2019年08月18日 19时45分22秒 CST
 ************************************************************************/

#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>

//event message
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
#define QUIT_EVENT  (SDL_USEREVENT + 2)


int thread_exit = 0;


int refresh_video_timer(void *udata)
{
	thread_exit = 0;
	SDL_Log("E: refresh_video_timer----->thread_exit: %d\n",thread_exit);
	while (!thread_exit) {
		SDL_Event event;
		event.type = REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}
	
	thread_exit = 0;
	//push quit event
	SDL_Event event;
	event.type = QUIT_EVENT;
	SDL_PushEvent(&event);
	SDL_Log("X: refresh_video_timer----->thread_exit: %d\n",thread_exit);
	return 0;
}

int main(int argc, char* argv[])
{
	if(argc < 4){
		SDL_Log("Usage %s w h input.yuv\n",argv[0]);
		return -1;
	}
	FILE *video_fd = NULL;
	SDL_Event event;
	SDL_Rect rect;

	Uint32 pixformat = 0;
	
	SDL_Window *win = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;

	SDL_Thread *timer_thread = NULL;
	
	int w_width = 1280, w_height = 960;
	const int video_width = atoi(argv[1]), video_height = atoi(argv[2]);
	
	Uint8 *video_pos = NULL;
	Uint8 *video_end = NULL;

	unsigned int remain_len = 0;
	unsigned int video_buff_len = 0;
	unsigned int blank_space_len = 0;
	Uint8* video_buf;

	const char* path = argv[3];
	const unsigned int yuv_frame_len = video_width * video_height * 12 / 8;
	unsigned int tmp_yuv_frame_len = yuv_frame_len;

	if(yuv_frame_len & 0xF){
		tmp_yuv_frame_len = (yuv_frame_len & 0xFFF0) + 0x10;
	}
    SDL_Log("tmp_yuv_frame_len: %d yuv_frame_len: %d\n",tmp_yuv_frame_len,yuv_frame_len);

	//initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	//creat window from SDL
	win = SDL_CreateWindow("YUV Player",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, 
			                w_width, w_height,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!win) {
		SDL_Log("Failed to create window, %s\n",SDL_GetError());
		goto __FAIL;
	}
	
	renderer = SDL_CreateRenderer(win, -1, 0);

	//I420: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	pixformat = SDL_PIXELFORMAT_IYUV;

	//create texture for renderer
	texture = SDL_CreateTexture(renderer,pixformat,SDL_TEXTUREACCESS_STREAMING,
			                    video_width,video_height);
	
	video_buf = (Uint8*)malloc(tmp_yuv_frame_len);
	if(!video_buf){
		SDL_Log("Failed to alloce yuv frame space!\n");
		goto __FAIL;
	}

	//open yuv File
	video_fd = fopen(path, "r");
	if(!video_fd ){
		SDL_Log("Failed to open yuv file\n");
		goto __FAIL;
	}
	
	//read block data
	if((video_buff_len = fread(video_buf, 1, yuv_frame_len, video_fd)) <= 0){
		SDL_Log("Failed to read data from yuv file!\n");
		goto __FAIL;
	}

	//set video positon
	video_pos = video_buf;

	timer_thread = SDL_CreateThread(refresh_video_timer,NULL,NULL);

	do{
		//Wait
		//SDL_Log("wait event ....\n");
		SDL_WaitEvent(&event);
		//SDL_Log("event type: %d\n",event.type);
		if(event.type == REFRESH_EVENT){
			SDL_UpdateTexture(texture, NULL, video_pos, video_width);

			//If window is resize
		    rect.x = 0;
			rect.y = 0;
			rect.w = w_width;
			rect.h = w_height;
			
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, &rect);
			SDL_Log("refresh event\n");
			SDL_RenderPresent(renderer);
            
			if((video_buff_len = fread(video_buf, 1, yuv_frame_len, video_fd)) <= 0){
				SDL_Log("read end of file\n");
				thread_exit = 1;
				continue;
			}
			video_pos = video_buf;
		}else if(event.type == SDL_WINDOWEVENT){
			//If Resize
			SDL_GetWindowSize(win, &w_width, &w_height);
		}else if(event.type == SDL_QUIT){
			 SDL_Log("quit event\n");
			 thread_exit = 1;
		}else if(event.type == QUIT_EVENT){
			SDL_Log("sdl quit\n");
			break;
		}
	}while(1);

__FAIL:
    if(video_buf){
		free(video_buf);
	}
	//close File
	if(video_fd){
		fclose(video_fd);
	}

	if(texture){
		SDL_DestroyTexture(texture);
		texture = NULL;
	}

	if(renderer){
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}

	if(win){
		SDL_DestroyWindow(win);
		win = NULL;
	}
	SDL_Quit();

	return 0;
}



