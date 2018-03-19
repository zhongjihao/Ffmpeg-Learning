remuxer : simplest_ffmpeg_remuxer.o
	g++ simplest_ffmpeg_remuxer.o  -oremuxer -I /usr/local/include -L /usr/local/lib -lavformat -lavcodec -lavutil

simplest_ffmpeg_remuxer.o : simplest_ffmpeg_remuxer.cpp
	g++ -c -fpic simplest_ffmpeg_remuxer.cpp -g -o simplest_ffmpeg_remuxer.o


.Python : clean
clean :
	@rm -f *.o *.mp4 remuxer

