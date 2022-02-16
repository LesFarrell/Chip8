// MIT License
//
// Copyright (c) 2018 Les Farrell
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE


// Chip8 Memory map
// 0x000-0x1FF - Chip 8 interpreter
// 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
// 0x200-0xFFF - Program ROM and work RAM

#ifndef CHIP8_HEADER
    #define CHIP8_HEADER

    #define BACKGROUND BLUE
    #define FOREGROUND WHITE
    #define SCREENWIDTH 64
    #define SCREENHEIGHT 32


    // Current Chip8_OpCode
    unsigned short  Chip8_OpCode;                       // Current Chip8_OpCode

    // Program Memory
    unsigned char   Chip8_ProgramMemory[4096];          // Chip 8's Main Memory

    // Display Memory
    unsigned char   Chip8_DisplayMemory[2048];          // Chip 8 Display (64 * 32)

    // Various Registers
    unsigned char   Chip8_Registers[16];                // Chip8's 16 Registers
    unsigned short  Chip8_IndexRegister;                // Chip8's Index register
    unsigned short  Chip8_ProgramCounter;               // Program counter

    // Timers
    unsigned char   Chip8_DelayTimer;                   // Delay Timer
    unsigned char   Chip8_SoundTimer;                   // Sound Timer

    // Chip8 Call Stack and Pointer
    unsigned short  Chip8_Stack[16];                    // Chip8's stack
    unsigned short  Chip8_StackPointer;                 // Stack pointer

    // Chip8 Key States
    unsigned char   Chip8_KeyStates[16];                // Store key states

    // Screen Update Flag
    unsigned char   Chip8_DrawFlag;                     // Okay to redraw screen

    // Chip 8 Default Fonts
    unsigned char Chip8_FontSet[80] =  {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    // Our Default Chip8 ROM
    unsigned char Chip8_LogoRom[] = {
        0,224,162,72,96,0,97,30,98,0,210,2,210,18,114,8,50,64,18,10,96,0,97,62,98,2,162,74,208,46,209,46,114,14,208,46,
        209,46,162,88,96,11,97,8,208,31,112,10,162,103,208,31,112,10,162,118,208,31,112,3,162,133,208,31,112,10,162,148,
        208,31,18,70,255,255,192,192,192,192,192,192,192,192,192,192,192,192,192,192,255,128,128,128,128,128,128,128,128,
        128,128,128,128,128,255,129,129,129,129,129,129,129,255,129,129,129,129,129,129,129,128,128,128,128,128,128,128,
        128,128,128,128,128,128,128,128,255,129,129,129,129,129,129,255,128,128,128,128,128,128,128,255,129,129,129,129,
        129,129,255,129,129,129,129,129,129,255,255
    };




    // Function prototypes
    void Chip8_LoadDefaultROM(void);
    int Chip8_LoadROM(char *ROM_FileName);
    void Chip8_Initialise(void);
    void Chip8_EmulateCPU(void);
    void Chip8_GetKeyStates(void);
    void Chip8_DrawScreen(void);
    void Chip8_ShowProgramState(void);    
    void Chip8_ProcessDroppedFiles(void);

#endif
