#include "console.h"
#include "windows.h"

HANDLE consoleHandle = NULL;

///
/// # Console_Show
/// Shows a console window, suitable for user interactions.
///
/// **Parameters:**
///
///     String : The title of the console window.
///
/// **Returns:**
///
///     Nil
///
/// **Notes:**
///
int Console_Show(const char *title) {
    AllocConsole();
    AttachConsole(GetCurrentProcessId());

    freopen("CON", "w", stdout);
    SetConsoleTitleA(title);
    consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    Console_Clear();
    return 0;
}


//----------------------------------------------------------------------------------------


///
/// # Console_Hide
/// Stop showing console window.
///
/// **Parameters:**
///
///     None :
///
/// **Returns:**
///
///     Nil
///
/// **Notes:**
///
int Console_Hide() {
    FreeConsole();
    consoleHandle = NULL;
    return 0;
}


//----------------------------------------------------------------------------------------


///
/// # Console_SetXY
/// Set the cursor position within the displayed console window.
///
/// **Parameters:**
///
///     Number : X coordinate.
///
///     Number : Y coordinate.
///
/// **Returns:**
///
///     Nil
///
/// **Notes:**
///
int Console_SetXY(int X, int Y) {
    COORD pos;

    if (consoleHandle == NULL) return 0;
    pos.X = X;
    pos.Y = Y;
    SetConsoleCursorPosition(consoleHandle, pos);

    return 0;
}


///
/// # Console_TextXY
/// Set the cursor position and write the passed string.
///
/// **Parameters:**
///
///     String : Text to display.
///
///     Number : X coordinate.
///
///     Number : Y coordinate.
///
/// **Returns:**
///
///     Nil
///
/// **Notes:**
///
int Console_TextXY(const char *string, int X, int Y)
{
    COORD pos;

    if (consoleHandle == NULL) return 0;
    pos.X = X;
    pos.Y = Y;
    SetConsoleCursorPosition(consoleHandle, pos);
    printf("%s",string);
    return 0;
}

//----------------------------------------------------------------------------------------


///
/// # Console_SetTextColour
/// Set the colour and background of the text in the console.
///
/// **Parameters:**
///
///     Number : Colour to use for text.
///              (Background Colour * 16 + Foreground Colour)
///
/// **Returns:**
///
///     Nil
///
/// **Notes:**
///
/// | Description    | Colour       |
/// |----------------|--------------|
/// | Black          | 0            |
/// | Blue           | 1            |
/// | Green          | 2            |
/// | Cyan           | 3            |
/// | Red            | 4            |
/// | Magenta        | 5            |
/// | Yellow         | 6            |
/// | White          | 7            |
/// | Grey           | 8            |
/// | Light Blue     | 9            |
/// | Light Green    | 10           |
/// | Light Cyan     | 11           |
/// | Light Red      | 12           |
/// | Light Magenta  | 13           |
/// | Light Yellow   | 14           |
/// | Light White    | 15           |
///
int Console_TextColour(int TextColour) {
    if (consoleHandle == NULL) return 0;
    SetConsoleTextAttribute(consoleHandle, (WORD) TextColour);
    return 0;
}



///
/// # Console_SetColour
/// Set the foreground and background colour of the text in the console.
///
/// **Parameters:**
///
///     Number : Foreground colour for text.
///
///     Number : Background colour for text.
///
/// **Returns:**
///
///     Nil
///
/// **Notes:**
///
/// | Description    | Colour       |
/// |----------------|--------------|
/// | Black          | 0            |
/// | Blue           | 1            |
/// | Green          | 2            |
/// | Cyan           | 3            |
/// | Red            | 4            |
/// | Magenta        | 5            |
/// | Yellow         | 6            |
/// | White          | 7            |
/// | Grey           | 8            |
/// | Light Blue     | 9            |
/// | Light Green    | 10           |
/// | Light Cyan     | 11           |
/// | Light Red      | 12           |
/// | Light Magenta  | 13           |
/// | Light Yellow   | 14           |
/// | Light White    | 15           |
///
int Console_SetColour(int foreground, int background) {

    if (consoleHandle == NULL) return 0;
    SetConsoleTextAttribute(consoleHandle, background * 16 + foreground);
    return 1;
}



//----------------------------------------------------------------------------------------


///
/// # Console_Clear
/// Clears the contents of the console window.
///
/// **Parameters:**
///
///     None
///
/// **Returns:**
///
///     Nil
///
/// **Notes:**
///
int Console_Clear() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD coordScreen = {0, 0};
    DWORD cCharsWritten;
    DWORD dwConSize;

    //if (consoleHandle == NULL) { return 0; }

    // Get the number of character cells in the current buffer.
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) return 0;

    // Calculate the size of the console.
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // Fill the entire screen with blanks.
    if (!FillConsoleOutputCharacter(consoleHandle, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten)) return 0;

    // Get the current text attribute.
    if (!GetConsoleScreenBufferInfo(consoleHandle, &csbi)) {
        return 0;
    }

    // Set the buffer's attributes accordingly.
    if (!FillConsoleOutputAttribute(consoleHandle, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten)) return 0;

    // Put the cursor at its home coordinates.
    SetConsoleCursorPosition(consoleHandle, coordScreen);

    return 1;
}





///
/// # Console_SetSize
/// Set the width and height of the console window in characters
///
/// **Parameters:**
///
///     Number: Width in characters
///
///     Number: Height in characters
///
/// **Returns:**
///
///     Number: If the function succeeds, the return value is nonzero.
///
/// **Notes:**
///
///     ```
///     console_setsize(w,h)
///     ```
///
int Console_SetSize(int width,int height) {
    COORD  dwSize;
    int result = 0;
    dwSize.X = width;
    dwSize.Y = height;
    result = SetConsoleScreenBufferSize(consoleHandle,  dwSize);
    return result;
}
