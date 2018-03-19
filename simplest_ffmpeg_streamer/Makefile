
all : pushstream pullstream

pushstream : simplest_ffmpeg_send_streamer.o
	g++ simplest_ffmpeg_send_streamer.o  -opushstream -I /usr/local/include -L /usr/local/lib -lavformat -lavcodec -lavutil

simplest_ffmpeg_send_streamer.o : simplest_ffmpeg_send_streamer.cpp
	g++ -c -fpic simplest_ffmpeg_send_streamer.cpp -g -o simplest_ffmpeg_send_streamer.o

pullstream : simplest_ffmpeg_recv_streamer.o
	g++ simplest_ffmpeg_recv_streamer.o -opullstream -I /usr/local/include -L /usr/local/lib -lavformat -lavcodec -lavutil

simplest_ffmpeg_recv_streamer.o : simplest_ffmpeg_recv_streamer.cpp
	g++ -c -fpic simplest_ffmpeg_recv_streamer.cpp -g -o simplest_ffmpeg_recv_streamer.o

.Python : clean
clean :
	@rm -f *.o pushstream pullstream

