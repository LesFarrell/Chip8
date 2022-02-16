tcc -O3 chip8.c -std=c99 -lmsvcrt -lraylib -lopengl32 -lgdi32 -lkernel32 -lshell32 -luser32 -lcomdlg32 -Wl,-subsystem=gui console.c filedialogs.c -o chip8.exe
rem tcc -O3 chip8.c -std=c99 -lmsvcrt -lraylib -lopengl32 -lgdi32 -lkernel32 -lshell32 -luser32 -lcomdlg32 console.c filedialogs.c -o chip8.exe
pause

