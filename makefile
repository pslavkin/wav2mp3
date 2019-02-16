all:
	g++ main.cpp -lpthread -o wav2mp3

clean:
	rm t.wav/*.mp3
