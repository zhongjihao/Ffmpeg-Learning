demuxer_simple : simplest_ffmpeg_demuxer_simple.o
	g++ simplest_ffmpeg_demuxer_simple.o  -odemuxer_simple -I /usr/local/include -L /usr/local/lib -lavformat -lavcodec -lavutil

simplest_ffmpeg_demuxer_simple.o : simplest_ffmpeg_demuxer_simple.cpp
	g++ -c -fpic simplest_ffmpeg_demuxer_simple.cpp -g -o simplest_ffmpeg_demuxer_simple.o


.Python : clean
clean :
	@rm -f *.o *.mp3 *.h264 demuxer_simple
	clear

