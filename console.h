#ifndef CONSOLE_HEADER
#define CONSOLE_HEADER

#include <stdio.h>
#include <stdlib.h>

int Console_Show(const char *title);
int Console_Hide();
int Console_SetXY(int X, int Y);
int Console_TextXY(const char *string, int X, int Y);
int Console_TextColour(int TextColour);
int Console_SetColour(int foreground, int background);
int Console_Clear();
int Console_SetSize(int width,int height);
#endif