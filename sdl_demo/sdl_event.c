/*************************************************************************
    > File Name: sdl_event.c
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2019年08月18日 15时32分36秒 CST
 ************************************************************************/

#include <stdio.h>
#include <SDL.h>

int main(int argc,char* argv[])
{
	int quit = 1;
	SDL_Window* window = NULL;
    SDL_Renderer* render = NULL;
	SDL_Event event;

	SDL_Init(SDL_INIT_VIDEO);
    
	window = SDL_CreateWindow("SDL2 Window",200,200,640,480,SDL_WINDOW_SHOWN);
    if(!window)
	{
		SDL_Log("Failed to Created window!\n");
		goto __EXIT;
	}

    render = SDL_CreateRenderer(window,-1,0);
	if(!render)
	{
		SDL_Log("Failed to Create render\n");
		goto __DWINDOW;
	}

	SDL_SetRenderDrawColor(render,255,0,0,255);
	
	SDL_RenderClear(render);

	SDL_RenderPresent(render);

	do{
		SDL_WaitEvent(&event);
		switch(event.type){
			case SDL_QUIT:
				quit = 0;
				break;
			default:
				SDL_Log("evnet type is %d\n",event.type);
		}
	}while(quit);

   SDL_DestroyRenderer(render);

__DWINDOW:  
	SDL_DestroyWindow(window);

__EXIT:
	SDL_Quit();
	return 0;
}




