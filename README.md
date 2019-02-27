wav2mp3
------------------
Cconvert directory of .wav .WAV .wAV, etc music files to mp3 with predefined encoded options using pthread and lame libs
You'd compile for win (under cygwin) or linux. It'll work posix only funcs

Edit makefile and select between static or dynamically compiled version and then just

```
 make
```

and run

```
./exe/wav2mp3_static_linux disc0
```

On win you need to include the DLL's that I copy on exe dirs beside the .exe

TODO: on win, the statically compiled only include gcc and stdg++ but mp3lame is again dinamically compiled.
      on linux it works like a charm
