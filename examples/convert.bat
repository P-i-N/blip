@echo off
rem Converst all *.mml files in current directory into *.wav files first (using blipgen.exe)
rem and then into *.mp3 files (using FFmpeg)

set BLIPGEN_EXE=..\.bin\Release\blipgen.exe
set FFMPEG_EXE=ffmpeg.exe

for %%f in (*.mml) do (
	rem Convert *.mml into *.wav
	%BLIPGEN_EXE% -i %%f

	rem Convert *.wav into *.mp3, 64 kbps, mono, overwrite all
	%FFMPEG_EXE% -i %%~nf.wav -b:a 64k -ac 1 -y %%~nf.mp3

	rem Also create *.mp4 video file with the same audio
	%FFMPEG_EXE% -i %%~nf.wav -b:a 64k -ac 1 -y %%~nf.mp4
		
	rem Delete *.wav
	del %%~nf.wav
)

pause
