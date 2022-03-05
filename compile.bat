windres -i application.rc -o application.o
tcc -s -O3 -DTIGR_DO_NOT_PRESERVE_WINDOW_POSITION chip8.c tigr.c -std=c99 -lmsvcrt -lraylib -lopengl32 -lgdi32 -lkernel32 -lshell32 -luser32 -lcomdlg32 -Wl,-subsystem=gui console.c filedialogs.c application.o -o chip8.exe
REM tcc -g chip8.c tigr.c -std=c99 -lmsvcrt -lopengl32 -lgdi32 -lkernel32 -lshell32 -luser32 -lcomdlg32 console.c filedialogs.c -o chip8.exe


tcc -g png2sprite.c tigr.c -std=c99 -lmsvcrt -lopengl32 -lgdi32 -lkernel32 -lshell32 -luser32 -lcomdlg32 console.c filedialogs.c -Wl,-subsystem=console -o png2sprite.exe

pause

