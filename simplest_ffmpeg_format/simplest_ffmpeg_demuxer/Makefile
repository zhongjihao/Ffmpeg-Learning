demuxer : simplest_ffmpeg_demuxer.o
	g++ simplest_ffmpeg_demuxer.o  -odemuxer -I /usr/local/include -L /usr/local/lib -lavformat -lavcodec -lavutil

simplest_ffmpeg_demuxer.o : simplest_ffmpeg_demuxer.cpp
	g++ -c -fpic simplest_ffmpeg_demuxer.cpp -g -o simplest_ffmpeg_demuxer.o


.Python : clean
clean :
	@rm -f *.o *.aac *.h264 demuxer

