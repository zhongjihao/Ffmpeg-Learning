muxer : simplest_ffmpeg_muxer.o
	g++ simplest_ffmpeg_muxer.o  -omuxer -I /usr/local/include -L /usr/local/lib -lavformat -lavcodec -lavutil

simplest_ffmpeg_muxer.o : simplest_ffmpeg_muxer.cpp
	g++ -c -fpic simplest_ffmpeg_muxer.cpp -g -o simplest_ffmpeg_muxer.o


.Python : clean
clean :
	@rm -f *.o  muxer

