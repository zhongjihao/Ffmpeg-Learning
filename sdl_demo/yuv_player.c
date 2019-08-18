/*************************************************************************
    > File Name: yuv_player.c
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2019年08月18日 19时45分22秒 CST
 ************************************************************************/

#include <stdio.h>
#include <string.h>

#include <SDL.h>

#define BLOCK_SIZE 4096000

//event message
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
#define QUIT_EVENT  (SDL_USEREVENT + 2)


int thread_exit = 0;


int refresh_video_timer(void *udata)
{
	thread_exit = 0;
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
	
	return 0;
}

int main(int argc, char* argv[])
{
	FILE *video_fd = NULL;
	SDL_Event event;
	SDL_Rect rect;

	Uint32 pixformat = 0;
	
	SDL_Window *win = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;

	SDL_Thread *timer_thread = NULL;
	
	int w_width = 640, w_height = 480;
	const int video_width = 608, video_height = 368;
	
	Uint8 *video_pos = NULL;
	Uint8 *video_end = NULL;

	unsigned int remain_len = 0;
	unsigned int video_buff_len = 0;
	unsigned int blank_space_len = 0;
	Uint8 video_buf[BLOCK_SIZE];

    if(argc < 2){
		SDL_Log("Usage %s %s\n",argv[0],argv[1]);
		return -1;
	}

	const char* path = argv[1];
	const unsigned int yuv_frame_len = video_width * video_height * 12 / 8;

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

	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	pixformat= SDL_PIXELFORMAT_IYUV;

	//create texture for renderer
	texture = SDL_CreateTexture(renderer,pixformat,SDL_TEXTUREACCESS_STREAMING,
			                    video_width,video_height);
	
	//open yuv File
	video_fd = fopen(path, "r");
	if(!video_fd ){
		SDL_Log("Failed to open yuv file\n");
		goto __FAIL;
	}
	
	//read block data
	if((video_buff_len = fread(video_buf, 1, BLOCK_SIZE, video_fd)) <= 0){
		SDL_Log("Failed to read data from yuv file!\n");
		goto __FAIL;
	}

	//set video positon
	video_pos = video_buf;
	video_end = video_buf + video_buff_len;
	blank_space_len = BLOCK_SIZE - video_buff_len;

	timer_thread = SDL_CreateThread(refresh_video_timer,NULL,NULL);

	do{
		//Wait
		SDL_WaitEvent(&event);
		if(event.type == REFRESH_EVENT){
			//not enought data to render
			if((video_pos + yuv_frame_len) > video_end){
				//have remain data, but there isn't space
				remain_len = video_end - video_pos;
				if(remain_len && !blank_space_len) {
					//copy data to header of buffer
					memcpy(video_buf, video_pos, remain_len);
					blank_space_len = BLOCK_SIZE - remain_len;
					video_pos = video_buf;
					video_end = video_buf + remain_len;
				}

				//at the end of buffer, so rotate to header of buffer
				if(video_end == (video_buf + BLOCK_SIZE)){
					video_pos = video_buf;
					video_end = video_buf;
					blank_space_len = BLOCK_SIZE;
				}

				//read data from yuv file to buffer
				if((video_buff_len = fread(video_end, 1, blank_space_len, video_fd)) <= 0){
					SDL_Log("eof, exit thread!\n");
					thread_exit = 1;
					continue;// to wait event for exiting
				}

				//reset video_end
				video_end += video_buff_len;
				blank_space_len -= video_buff_len;
			    SDL_Log("not enought data: pos:%p, video_end:%p, blank_space_len:%d\n", 
						video_pos, video_end, blank_space_len);
			}

			SDL_UpdateTexture( texture, NULL, video_pos, video_width);

			//If window is resize
		    rect.x = 0;
			rect.y = 0;
			rect.w = w_width;
			rect.h = w_height;
			
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, &rect);
			SDL_RenderPresent(renderer);

		    SDL_Log("not enought data: pos:%p, video_end:%p, blank_space_len:%d\n", 
					video_pos, video_end, blank_space_len);
			video_pos += yuv_frame_len;
		}else if(event.type == SDL_WINDOWEVENT){
			//If Resize
			SDL_GetWindowSize(win, &w_width, &w_height);
		}else if(event.type == SDL_QUIT){
			 thread_exit = 1;
		}else if(event.type == QUIT_EVENT){
			break;
		}
	}while(1);

__FAIL:
	//close File
	if(video_fd){
		fclose(video_fd);
	}

	SDL_Quit();

	return 0;
}



