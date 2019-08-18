/*************************************************************************
    > File Name: sdl_texture.c
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
	SDL_Texture* texture = NULL;
	
	SDL_Rect rect;
	rect.w = 30;
	rect.h = 30;

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
 
	//创建纹理
	texture = SDL_CreateTexture(render,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,640,480);
    if(!texture){
		SDL_Log("Failed to Created Texture!\n");
		goto __DRENDER;
	}

	do{
		SDL_PollEvent(&event);
		switch(event.type){
			case SDL_QUIT:
				quit = 0;
				break;
			default:
				SDL_Log("evnet type is %d\n",event.type);
		}

		rect.x = rand() % 600;
		rect.y = rand() % 450;

		//设置渲染目标
		SDL_SetRenderTarget(render,texture);
        //设置纹理背景颜色
		SDL_SetRenderDrawColor(render,0,0,0,0);
        //刷新纹理
		SDL_RenderClear(render);

		//绘制矩形
		SDL_RenderDrawRect(render,&rect);
		SDL_SetRenderDrawColor(render,255,0,0,0);
        //矩形填充颜色
		SDL_RenderFillRect(render,&rect);

		//恢复默认渲染目标即窗口
		SDL_SetRenderTarget(render,NULL);
		//将纹理输出到显卡显示
		SDL_RenderCopy(render,texture,NULL,NULL);

		//显示渲染后的结果
		SDL_RenderPresent(render);
	}while(quit);
    
	//销毁纹理
	SDL_DestroyTexture(texture);

__DRENDER:
	SDL_DestroyRenderer(render);

__DWINDOW:  
	SDL_DestroyWindow(window);

__EXIT:
	SDL_Quit();
	return 0;
}




