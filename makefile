all:
ifeq ($(OS),Windows_NT)
		g++ -v -static main.cpp -lpthread -L./static_lib/win/ -lmp3lame -o wav2mp3
else
		g++ -v main.cpp -lpthread   -Wl,-static -L./static_lib/linux/ -lmp3lame  -Wl,-Bdynamic -o wav2mp3

endif
