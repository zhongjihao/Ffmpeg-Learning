SDL基本使用 \
SDL下载地址 \
	http://www.libsdl.org/download-2.0.php

1 SDL创建窗口显示例子sdl_window.c \
  gcc -g -o sdlwindow sdl_window.c `pkg-config --cflags --libs sdl2`

2 SDL事件处理例子sdl_event.c \
   gcc -g -o sdlevent sdl_event.c `pkg-config --cflags --libs sdl2`

3 SDL纹理渲染例子sdl_texture.c \
  gcc -g -o sdltexture sdl_texture.c `pkg-config --cflags --libs sdl2`

4 yuv播放器简单例子yuv_player.c \
  gcc -g -o yuv_play yuv_player.c `pkg-config --cflags --libs sdl2`

5 yuv叠加生成水印yuv_overlay.c \
  gcc -o yuvwater yuv_overlay.c -DDUMP_OUTPUT

6 pcm播放器简单例子pcm_player.c \
  gcc -g -o pcm_player pcm_player.c `pkg-config --cflags --libs sdl2`

