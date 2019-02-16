all:
	g++ main.cpp -lpthread -lmp3lame -o wav2mp3

clean:
	rm t.wav/*.mp3
