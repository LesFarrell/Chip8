#include "filedialogs.h"
#include <windows.h>

int OpenFileDialog(char *buffer, long buffersize)
{

    OPENFILENAME ofn = {0};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = buffersize;
    ofn.lpstrFilter = "Chip8 ROM files\0*.ch8";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        return 1;
    }
    else
    {
        // Return empty buffer if nothing selected.
        buffer[0] = '\0';
        return 0;
    }
}
