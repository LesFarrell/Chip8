windres -i application.rc -o application.o
tcc -s -O3 application.o chip8.c tigr.c console.c filedialogs.c -lmsvcrt -lopengl32 -lgdi32 -lkernel32 -lshell32 -luser32 -lcomdlg32 -Wl,-subsystem=gui -o SuperChip.exe
REM tcc -s -O3 -DNDEBUG -DTIGR_DO_NOT_PRESERVE_WINDOW_POSITION application.o chip8.c tigr.c console.c filedialogs.c -lmsvcrt -lopengl32 -lgdi32 -lkernel32 -lshell32 -luser32 -lcomdlg32 -Wl,-subsystem=gui -o SuperChip.exe
pause
