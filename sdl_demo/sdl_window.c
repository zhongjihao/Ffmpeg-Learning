/*************************************************************************
    > File Name: sdl_window.c
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2019年08月18日 15时32分36秒 CST
 ************************************************************************/

#include <stdio.h>
#include <SDL.h>

int main(int argc,char* argv[])
{
	SDL_Window* window = NULL;
    SDL_Renderer* render = NULL;

	SDL_Init(SDL_INIT_VIDEO);
    
	window = SDL_CreateWindow("SDL2 Window",200,200,640,480,SDL_WINDOW_SHOWN);
    if(!window)
	{
		SDL_Log("Failed to Created window!\n");
		goto __EXIT;
	}

	//创建渲染器
    render = SDL_CreateRenderer(window,-1,0);
	if(!render)
	{
		SDL_Log("Failed to Create render\n");
		goto __DWINDOW;
	}

	SDL_SetRenderDrawColor(render,255,0,0,255);
	
	SDL_RenderClear(render);

	SDL_RenderPresent(render);

	SDL_Delay(10*1000); //delay 10s

	SDL_DestroyRenderer(render);

__DWINDOW:  
	SDL_DestroyWindow(window);

__EXIT:
	SDL_Quit();
	return 0;
}




