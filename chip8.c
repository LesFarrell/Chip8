// MIT License
//
// Copyright (c) 2018 - 2022 Les Farrell
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

#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <string.h>
#include <time.h>
#include "chip8.h"
#include "console.h"
#include "filedialogs.h"

double LastChip8_DelayUpdate = 0;
double LastSoundUpdate = 0;

//------------------------------------------------------------------------------

void Chip8_Initialise(void)
{
    Chip8_ProgramCounter = 0x200; // Program counter.
    Chip8_IndexRegister = 0;      // Chip8_IndexRegister register.
    Chip8_DelayTimer = 0;         // Chip8_DelayTimer timer.
    Chip8_SoundTimer = 0;         // Sound timer.
    Chip8_StackPointer = 0;       // Stack pointer.
    Chip8_DrawFlag = 0;           // Reset screen update flag.
    Chip8_OpCode = 0;             // Current op code.

    // Clear the display memory.
    for (int i = 0; i < 2048; ++i)
    {
        Chip8_DisplayMemory[i] = 0;
    }

    // Clear stack memory.
    for (int i = 0; i < 16; ++i)
    {
        Chip8_Stack[i] = 0;
    }

    // Clear the register states.
    for (int i = 0; i < 16; ++i)
    {
        Chip8_KeyStates[i] = Chip8_Registers[i] = 0;
    }

    // Clear main memory.
    for (int i = 0; i < 4096; ++i)
    {
        Chip8_ProgramMemory[i] = 0;
    }

    // Load the default Chip-8 font.
    for (int i = 0; i < 80; ++i)
    {
        Chip8_ProgramMemory[i] = Chip8_FontSet[i];
    }

    // Initialise the random seed.
    srand(time(NULL));
}

//------------------------------------------------------------------------------

void Chip8_DrawGraphics(void)
{
    int PixelWidth = 10;
    int PixelHeight = 10;

    // If the draw flag is set then update the screen
    if (Chip8_DrawFlag == 1)
    {
        for (int y = 0; y < 32; ++y)
        {
            for (int x = 0; x < 64; ++x)
            {
                if (Chip8_DisplayMemory[x + (y * 64)] != 0)
                {
                    DrawRectangle(x * PixelWidth, y * PixelHeight, PixelWidth, PixelHeight, WHITE);
                }
                else
                {
                    DrawRectangle(x * PixelWidth, y * PixelHeight, PixelWidth, PixelHeight, BLUE);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------

int Chip8_LoadROM(char *ROM_FileName)
{
    FILE *fp;
    unsigned short pos = 512;
    unsigned char ch;

    fp = fopen(ROM_FileName, "rb");
    if (fp == NULL)
    {
        return 0;
    }

    while (!feof(fp))
    {
        ch = fgetc(fp);
        Chip8_ProgramMemory[pos] = ch;
        pos++;
        if (pos >= 4096)
        {
            break;
        }
    }
    fclose(fp);
    return 0;
}

//------------------------------------------------------------------------------

void Chip8_LoadLogo()
{
    unsigned short pos = 512;
    for (int loop = 0; loop < sizeof(Chip8_LogoRom); loop++)
    {
        Chip8_ProgramMemory[pos++] = Chip8_LogoRom[loop];
    }
}

//------------------------------------------------------------------------------

void Chip8_ShowProgramState(void)
{

    int i = 0;
    int y = 40;
    char string[100];
    char buffer[1024] = {'\0'};

    Console_SetXY(1, 1);

    sprintf(string, "PC : %d\t", Chip8_ProgramCounter);
    Console_TextXY(string, 1, 1);

    sprintf(string, "SP : %d\t", Chip8_StackPointer);
    Console_TextXY(string, 1, 2);

    sprintf(string, "Index : %d\t", Chip8_IndexRegister);
    Console_TextXY(string, 1, 3);

    sprintf(string, "Delay : %d\t", Chip8_DelayTimer);
    Console_TextXY(string, 1, 4);

    sprintf(string, "Sound : %d\t", Chip8_SoundTimer);
    Console_TextXY(string, 1, 5);

    y = 7;
    for (i = 0; i < 16; i++)
    {
        sprintf(string, "V%02d : %02d\t", i, Chip8_Registers[i]);
        Console_TextXY(string, 1, y);
        y = y + 1;
    }

    y = 7;
    for (i = 0; i < 16; i++)
    {
        sprintf(string, "Stack[%02d] : %02d\t", i, Chip8_Stack[i]);
        Console_TextXY(string, 20, y);
        y = y + 1;
    }

    sprintf(string, "[%d]:\t%s\t\t", Chip8_ProgramCounter, buffer);
    Console_TextXY(string, 1, 25);
}

//------------------------------------------------------------------------------

void Chip8_EmulateCPU(void)
{
    int x = 0, y = 0;
    int kk = 0, nnn = 0;
    unsigned short height = 0;
    unsigned short pixel = 0;
    char buffer[1024] = {'\0'};

    double CurrentTime = GetTime();

    // Grab the next Chip8_OpCode.
    Chip8_OpCode = ((Chip8_ProgramMemory[Chip8_ProgramCounter] << 8) + Chip8_ProgramMemory[Chip8_ProgramCounter + 1]);

    // Process the Chip8_OpCode.
    switch (Chip8_OpCode & 0xF000)
    {
    case 0x0000:
        switch (Chip8_OpCode & 0x000F)
        {

        // 00E0 - CLS
        // Clear the display.
        case 0x0000:
            sprintf(buffer, "%X\t : 00E0\t - \tCLS", Chip8_OpCode);
            for (int c = 0; c < 2048; c++)
            {
                Chip8_DisplayMemory[c] = 0;
            }
            Chip8_DrawFlag = 1;
            Chip8_ProgramCounter += 2;
            break;

        // 00EE - RET
        // Return from a subroutine.
        // The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
        case 0x000E:
            sprintf(buffer, "%X\t\t : 00EE\t - \tRET", Chip8_OpCode);
            Chip8_StackPointer--;
            Chip8_ProgramCounter = Chip8_Stack[Chip8_StackPointer];
            Chip8_ProgramCounter += 2;
            break;

        default:
            sprintf(buffer, "Unknown Chip8_OpCode [0x0000]: 0x%X\n", Chip8_OpCode);
        }
        break;

    // 1nnn - JP addr
    // Jump to location nnn.
    // The interpreter sets the program counter to nnn.
    case 0x1000:
        nnn = (Chip8_OpCode & 0x0FFF);
        sprintf(buffer, "%X\t : 1nnn\t - \tJP %d", Chip8_OpCode, nnn);
        Chip8_ProgramCounter = nnn;
        break;

    // 2nnn - CALL addr
    // Call subroutine at nnn.
    // The interpreter increments the stack pointer, then puts the Chip8_ProgramCounter value the top of the stack.
    // The Chip8_ProgramCounter is then set to nnn.
    case 0x2000:
        nnn = (Chip8_OpCode & 0x0FFF);
        sprintf(buffer, "%X\t : 2nnn\t - \tCALL %d", Chip8_OpCode, nnn);
        Chip8_Stack[Chip8_StackPointer] = Chip8_ProgramCounter;
        Chip8_StackPointer++;
        Chip8_ProgramCounter = nnn;
        break;

    // 3xkk - SE Vx, byte
    // Skip next instruction if Vx = kk.
    // The interpreter compares register Vx to kk, and if they are equal,
    // increments the program counter by 2.
    case 0x3000:
        x = (Chip8_OpCode & 0x0F00) >> 8;
        kk = (Chip8_OpCode & 0x00FF);
        sprintf(buffer, "%X\t : 3xkk\t - \tSE V%d, %d", Chip8_OpCode, x, kk);
        if (Chip8_Registers[x] == kk)
        {
            Chip8_ProgramCounter += 2;
        }
        Chip8_ProgramCounter += 2;
        break;

    // 4xkk - SNE Vx, byte
    // Skip next instruction if Vx != kk.
    // The interpreter compares register Vx to kk and if they are not equal,
    // increments the program counter by 2.
    case 0x4000:
        x = (Chip8_OpCode & 0x0F00) >> 8;
        kk = (Chip8_OpCode & 0x00FF);
        sprintf(buffer, "%X\t : 4xkk\t - \tSNE V%d, %d", Chip8_OpCode, x, kk);
        if (Chip8_Registers[x] != kk)
        {
            Chip8_ProgramCounter += 2;
        }
        Chip8_ProgramCounter += 2;
        break;

    // 5xy0 - SE Vx, Vy
    // Skip next instruction if Vx = Vy.
    // The interpreter compares register Vx to register Vy and if they are equal,
    // increments the program counter by 2.
    case 0x5000:
        x = (Chip8_OpCode & 0x0F00) >> 8;
        y = (Chip8_OpCode & 0x00F0) >> 4;
        sprintf(buffer, "%X\t : 5xy0\t - \tSE V%d, V%d", Chip8_OpCode, x, y);
        if (Chip8_Registers[x] == Chip8_Registers[y])
        {
            Chip8_ProgramCounter += 2;
        }
        Chip8_ProgramCounter += 2;
        break;

    // 6xkk - LD Vx, byte
    // Set Vx = kk.
    // The interpreter puts the value kk into register Vx.
    case 0x6000:
        x = (Chip8_OpCode & 0x0F00) >> 8;
        kk = (Chip8_OpCode & 0x00FF);
        sprintf(buffer, "%X\t : 6xkk\t - \tLD V%d, %d", Chip8_OpCode, x, kk);
        Chip8_Registers[x] = kk;
        Chip8_ProgramCounter += 2;
        break;

    // 7xkk - ADD Vx, byte
    // Set Vx = Vx + kk.
    // Adds the value kk to the value of register Vx, then stores the result in Vx.
    case 0x7000:
        x = (Chip8_OpCode & 0x0F00) >> 8;
        kk = (Chip8_OpCode & 0x00FF);
        sprintf(buffer, "%X\t : 7xkk\t - \tADD V%d, %d", Chip8_OpCode, x, kk);
        Chip8_Registers[x] += kk;
        Chip8_ProgramCounter += 2;
        break;

    case 0x8000:
        switch (Chip8_OpCode & 0x000F)
        {

        // 8xy0 - LD Vx, Vy
        // Set Vx = Vy.
        // stores the value of register Vy in register Vx.
        case 0x000:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            y = (Chip8_OpCode & 0x00F0) >> 4;
            sprintf(buffer, "%X\t : 8xy0\t - \tLD V%d, V%d", Chip8_OpCode, x, y);
            Chip8_Registers[x] = Chip8_Registers[y];
            Chip8_ProgramCounter += 2;
            break;

        // 8xy1 - OR Vx, Vy
        // Set Vx = Vx OR Vy.
        // Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
        // A bitwise OR compares the corresponding bits from two values,
        // and if either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0.
        case 0x001:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            y = (Chip8_OpCode & 0x00F0) >> 4;
            sprintf(buffer, "%X\t : 8xy1\t - \tOR V%d, V%d", Chip8_OpCode, x, y);
            Chip8_Registers[x] |= Chip8_Registers[y];
            Chip8_ProgramCounter += 2;
            break;

        // 8xy2 - AND Vx, Vy
        // Set Vx = Vx AND Vy.
        // Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
        // A bitwise AND compares the corresponding bits from two values,
        // and if both bits are 1, then the same bit in the result is also 1. Otherwise, it is 0.
        case 0x002:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            y = (Chip8_OpCode & 0x00F0) >> 4;
            sprintf(buffer, "%X\t : 8xy2\t - \tAND V%d, V%d", Chip8_OpCode, x, y);
            Chip8_Registers[x] &= Chip8_Registers[y];
            Chip8_ProgramCounter += 2;
            break;

        // 8xy3 - XOR Vx, Vy
        // Set Vx = Vx XOR Vy.
        // Performs a bitwise exclusive OR on the values of Vx and Vy,
        // then stores the result in Vx.
        // An exclusive OR compares the corresponding bits from two values,
        // and if the bits are not both the same then the corresponding bit
        // in the result is set to 1. Otherwise, it is 0.
        case 0x003:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            y = (Chip8_OpCode & 0x00F0) >> 4;
            sprintf(buffer, "%X\t : 8xy3\t - \tXOR V%d, V%d", Chip8_OpCode, x, y);
            Chip8_Registers[x] ^= Chip8_Registers[y];
            Chip8_ProgramCounter += 2;
            break;

        // 8xy4 - ADD Vx, Vy
        // Set Vx = Vx + Vy, set VF = carry.
        // The values of Vx and Vy are added together.
        // If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0.
        // Only the lowest 8 bits of the result are kept, and stored in Vx.
        case 0x0004:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            y = (Chip8_OpCode & 0x00F0) >> 4;
            sprintf(buffer, "%X\t : 8xy4\t - \tADD V%d, V%d", Chip8_OpCode, x, y);
            if (Chip8_Registers[y] > (255 - Chip8_Registers[x]))
            {
                Chip8_Registers[0xF] = 1;
            }
            else
            {
                Chip8_Registers[0xF] = 0;
            }
            Chip8_Registers[x] += Chip8_Registers[y];
            Chip8_ProgramCounter += 2;
            break;

        // 8xy5 - SUB Vx, Vy
        // Set Vx = Vx - Vy, set VF = NOT borrow.
        // If Vx > Vy, then VF is set to 1, otherwise 0.
        // Then Vy is subtracted from Vx, and the results stored in Vx.
        case 0x0005:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            y = (Chip8_OpCode & 0x00F0) >> 4;
            sprintf(buffer, "%X\t : 8xy5\t - \tSUB V%d, V%d", Chip8_OpCode, x, y);
            if (Chip8_Registers[x] > Chip8_Registers[y])
            {
                Chip8_Registers[0xF] = 1;
            }
            else
            {
                Chip8_Registers[0xF] = 0;
            }
            Chip8_Registers[x] -= Chip8_Registers[y];
            Chip8_ProgramCounter += 2;
            break;

        // 8xy6 - SHR Vx {, Vy}
        // Set Vx = Vx SHR 1.
        // If the least-significant bit of Vx is 1, then VF is set to 1,
        // otherwise 0. Then Vx is divided by 2.
        case 0x0006:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : 8xy6\t - \tSHR V%d {, V%d}", Chip8_OpCode, x, x);
            Chip8_Registers[0xF] = Chip8_Registers[x] & 0x1;
            Chip8_Registers[x] = Chip8_Registers[x] / 2;
            Chip8_ProgramCounter += 2;
            break;

        // 8xy7 - Subn Vx, Vy
        // Set Vx = Vy - Vx, set VF = NOT borrow.
        // If Vy > Vx, then VF is set to 1, otherwise 0.
        // Then Vx is subtracted from Vy, and the results stored in Vx.
        case 0x0007:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            y = (Chip8_OpCode & 0x00F0) >> 4;
            sprintf(buffer, "%X\t : 8Xy7\t - \tSubn V%d, V%d", Chip8_OpCode, x, y);
            if (Chip8_Registers[y] > Chip8_Registers[x])
            {
                Chip8_Registers[0xF] = 1;
            }
            else
            {
                Chip8_Registers[0xF] = 0;
            }
            Chip8_Registers[x] = Chip8_Registers[y] - Chip8_Registers[x];
            Chip8_ProgramCounter += 2;
            break;

        // 8xyE - SHL Vx {, Vy}
        // Set Vx = Vx SHL 1.
        // If the most-significant bit of Vx is 1, then VF is set to 1,
        // otherwise to 0. Then Vx is multiplied by 2.
        case 0x000E:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : 8xyE\t - \tSHL V%d {, V%d}", Chip8_OpCode, x, x);
            Chip8_Registers[0xF] = Chip8_Registers[x] >> 7;
            Chip8_Registers[x] = Chip8_Registers[x] * 2;
            Chip8_ProgramCounter += 2;
            break;
        }
        break;

    // 9xy0 - SNE Vx, Vy
    // Skip next instruction if Vx != Vy.
    // The values of Vx and Vy are compared and if they are not equal,
    // the program counter is increased by 2.
    case 0x9000:
        x = (Chip8_OpCode & 0x0F00) >> 8;
        y = (Chip8_OpCode & 0x00F0) >> 4;
        sprintf(buffer, "%X\t : 9xy0\t - \tSNE V%d, V%d", Chip8_OpCode, x, y);
        if (Chip8_Registers[x] != Chip8_Registers[y])
        {
            Chip8_ProgramCounter += 2;
        }
        Chip8_ProgramCounter += 2;
        break;

    // Annn - LD I, addr
    // Set Chip8_IndexRegister = nnn.
    // The value of register Chip8_IndexRegister is set to nnn.
    case 0xA000:
        nnn = (Chip8_OpCode & 0x0FFF);
        sprintf(buffer, "%X\t : Annn\t - \tLD I, %d", Chip8_OpCode, nnn);
        Chip8_IndexRegister = nnn;
        Chip8_ProgramCounter += 2;
        break;

    // Bnnn - JP V0, addr
    // Jump to location nnn + V0.
    // The program counter is set to nnn plus the value of V0.
    case 0xB000:
        nnn = (Chip8_OpCode & 0x0FFF);
        sprintf(buffer, "%X\t : Bnnn\t - \tJP V0, %d", Chip8_OpCode, nnn);
        Chip8_ProgramCounter = nnn + Chip8_Registers[0];
        break;

    // Cxkk - RND Vx, byte
    // Set Vx = random byte AND kk.
    // The interpreter generates a random number from 0 to 255,
    // which is then ANDed with the value kk. The results are stored in Vx.
    // See instruction 8xy2 for more information on AND.
    case 0xC000:
        x = (Chip8_OpCode & 0x0F00) >> 8;
        kk = (Chip8_OpCode & 0x00FF);
        sprintf(buffer, "%X\t : Cxkk\t - \tRND V%d, %d", Chip8_OpCode, x, kk);
        Chip8_Registers[x] = (rand() % 256) & kk;
        Chip8_ProgramCounter += 2;
        break;

    // Dxyn - DRW Vx, Vy, nibble
    // Display n-byte sprite starting at memory location Chip8_IndexRegister at (Vx, Vy), set VF = collision.
    // The interpreter reads n bytes from memory, starting at the address stored in I.
    // These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
    // Sprites are XORed onto the existing screen.
    // If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
    // If the sprite is positioned so part of it is outside the coordinates of the display,
    // it wraps around to the opposite side of the screen.
    case 0xD000:
        x = (Chip8_OpCode & 0x0F00) >> 8;
        y = (Chip8_OpCode & 0x00F0) >> 4;
        height = Chip8_OpCode & 0x000F;
        sprintf(buffer, "%X\t : Dxyn\t - \tDRW V%d, V%d, %d", Chip8_OpCode, x, y, height);
        pixel = 0;
        Chip8_Registers[0xF] = 0;
        for (int yline = 0; yline < height; yline++)
        {
            pixel = Chip8_ProgramMemory[Chip8_IndexRegister + yline];
            for (int xline = 0; xline < 8; xline++)
            {
                if ((pixel & (0x80 >> xline)) != 0)
                {
                    if (Chip8_DisplayMemory[(Chip8_Registers[x] + xline + ((Chip8_Registers[y] + yline) * 64))] == 1)
                    {
                        Chip8_Registers[0xF] = 1;
                    }
                    Chip8_DisplayMemory[Chip8_Registers[x] + xline + ((Chip8_Registers[y] + yline) * 64)] ^= 1;
                }
            }
        }
        Chip8_DrawFlag = 1;
        Chip8_ProgramCounter += 2;
        break;

    case 0xE000:
        switch (Chip8_OpCode & 0x00FF)
        {

        // Ex9E - SKP Vx
        // Skip next instruction if key with the value of Vx is pressed.
        // Checks the keyboard, and if the key corresponding to the value of Vx
        // is currently in the down position,
        // Chip8_ProgramCounter is increased by 2.
        case 0x009E:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : Ex9E\t - \tSKP V%d", Chip8_OpCode, x);
            if (Chip8_KeyStates[Chip8_Registers[x]] == 1)
            {
                Chip8_ProgramCounter += 2;
            }
            Chip8_ProgramCounter += 2;
            break;

        // ExA1 - SKNP Vx
        // Skip next instruction if key with the value of Vx is not pressed.
        // Checks the keyboard, and if the key corresponding to the value of Vx
        // is currently in the up position,
        // Chip8_ProgramCounter is increased by 2.
        case 0x00A1:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : ExA1\t - \tSKNP V%d", Chip8_OpCode, x);
            if (Chip8_KeyStates[Chip8_Registers[x]] == 0)
            {
                Chip8_ProgramCounter += 2;
            }
            Chip8_ProgramCounter += 2;
            break;
        }
        break;

    case 0xF000:
        switch (Chip8_OpCode & 0x00FF)
        {

        // Fx07 - LD Vx, Chip8_DelayTimer
        // Set Vx = Chip8_DelayTimer value.
        // The value of Chip8_DelayTimer is placed into Vx.
        case 0x0007:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : Fx07\t - \tLD V%d, Chip8_DelayTimer", Chip8_OpCode, x);
            Chip8_Registers[x] = Chip8_DelayTimer;
            Chip8_ProgramCounter += 2;
            break;

        // Fx0A - LD Vx, K
        // Wait for a key press, store the value of the key in Vx.
        // All execution stops until a key is pressed,
        // then the value of that key is stored in Vx.
        case 0x000A:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : Fx0A\t - \tLD V%d, K", Chip8_OpCode, x);
            int keyPress = 0;
            for (int i = 0; i < 16; i++)
            {
                if (Chip8_KeyStates[i] != 0)
                {
                    Chip8_Registers[x] = i;
                    keyPress = 1;
                }
            }
            // If we didn't received a keypress, skip this cycle and try again.
            if (!keyPress)
            {
                return;
            }
            Chip8_ProgramCounter += 2;
            break;

        // Fx15 - LD Chip8_DelayTimer, Vx
        // Set Chip8_DelayTimer = Vx.
        // Chip8_DelayTimer is set equal to the value of Vx.
        case 0x0015:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : Fx15\t - \tLD Chip8_DelayTimer, V%d", Chip8_OpCode, x);
            Chip8_DelayTimer = Chip8_Registers[x];
            Chip8_ProgramCounter += 2;
            break;

        // Fx18 - LD Chip8_SoundTimer, Vx
        // Set sound timer = Vx.
        // Chip8_SoundTimer is set equal to the value of Vx.
        case 0x0018:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : Fx18\t - \tLD Chip8_SoundTimer, V%d", Chip8_OpCode, x);
            Chip8_SoundTimer = Chip8_Registers[x];
            Chip8_ProgramCounter += 2;
            break;

        // Fx1E - ADD I, Vx
        // Set Chip8_IndexRegister = Chip8_IndexRegister + Vx.
        // The values of I and Vx are added, and the results are stored in I.
        case 0x001E:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : Fx1E\t - \tADD I, V%x", Chip8_OpCode, x);
            // VF is set to 1 when range overflow (Chip8_IndexRegister + VX > 0xFFF), and 0 when it isn't.
            if (Chip8_IndexRegister + Chip8_Registers[x] > 0xFFF)
            {
                Chip8_Registers[0xF] = 1;
            }
            else
            {
                Chip8_Registers[0xF] = 0;
            }
            Chip8_IndexRegister += Chip8_Registers[x];
            Chip8_ProgramCounter += 2;
            break;

        // Fx29 - LD F, Vx
        // Set Chip8_IndexRegister = location of sprite for digit Vx.
        // The value of Chip8_IndexRegister is set to the location for the hexadecimal sprite corresponding to the value of Vx.
        case 0x0029:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : Fx29\t - \tLD F, V%d", Chip8_OpCode, x);
            Chip8_IndexRegister = Chip8_Registers[x] * 0x5;
            Chip8_ProgramCounter += 2;
            break;

        // Fx33 - LD B, Vx
        // store BCD representation of Vx in memory locations Chip8_IndexRegister, Chip8_IndexRegister+1, and Chip8_IndexRegister+2.
        // The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in Chip8_IndexRegister,
        // the tens digit at location I+1, and the ones digit at location Chip8_IndexRegister + 2.
        case 0x0033:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : Fx33\t - \tLD B, V%d", Chip8_OpCode, x);
            Chip8_ProgramMemory[Chip8_IndexRegister] = Chip8_Registers[x] / 100;
            Chip8_ProgramMemory[Chip8_IndexRegister + 1] = (Chip8_Registers[x] / 10) % 10;
            Chip8_ProgramMemory[Chip8_IndexRegister + 2] = (Chip8_Registers[x] % 100) % 10;
            Chip8_ProgramCounter += 2;
            break;

        // Fx55 - LD [Chip8_IndexRegister], Vx
        // store registers V0 through Vx in memory starting at location Chip8_IndexRegister.
        // The interpreter copies the values of registers V0 through Vx into memory,
        // starting at the address in Chip8_IndexRegister.
        case 0x0055:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : Fx55\t - \tLD [I], V%d", Chip8_OpCode, x);
            for (int i = 0; i <= x; i++)
            {
                Chip8_ProgramMemory[Chip8_IndexRegister + i] = Chip8_Registers[i];
            }
            Chip8_ProgramCounter += 2;
            break;

        // Fx65 - LD Vx, [I]
        // Read registers V0 through Vx from memory starting at location I.
        // The interpreter reads values from memory starting at location I into registers V0 through Vx.
        case 0x0065:
            x = (Chip8_OpCode & 0x0F00) >> 8;
            sprintf(buffer, "%X\t : Fx65\t - \tLD V%d, [I]", Chip8_OpCode, x);
            for (int i = 0; i <= x; i++)
            {
                Chip8_Registers[i] = Chip8_ProgramMemory[Chip8_IndexRegister + i];
            }
            Chip8_ProgramCounter += 2;
            break;
        }
        break;

    default:
        printf("Unknown Chip8 OpCode %X\n", Chip8_OpCode);
        break;
    }

    // Is it time to decrement the Chip8_DelayTimer.
    if (CurrentTime - LastChip8_DelayUpdate > 0.0166)
    {
        if (Chip8_DelayTimer > 0)
        {
            Chip8_DelayTimer--;
        }
        LastChip8_DelayUpdate = CurrentTime;
    }

    // Is it time to decrement the Chip8_SoundTimer.
    if (CurrentTime - LastSoundUpdate > 0.0166)
    {
        if (Chip8_SoundTimer > 0)
        {
            Chip8_SoundTimer = Chip8_SoundTimer - 1;
        }
        LastSoundUpdate = CurrentTime;
    }
}

//------------------------------------------------------------------------------

void Chip8_GetKeyStates(void)
{
    if (IsKeyDown(KEY_ONE) || IsKeyPressed(KEY_ONE))
    {
        Chip8_KeyStates[0x1] = 1;
    }
    else
    {
        Chip8_KeyStates[0x1] = 0;
    }

    if (IsKeyDown(KEY_TWO) || IsKeyPressed(KEY_TWO))
    {
        Chip8_KeyStates[0x2] = 1;
    }
    else
    {
        Chip8_KeyStates[0x2] = 0;
    }

    if (IsKeyDown(KEY_THREE) || IsKeyPressed(KEY_THREE))
    {
        Chip8_KeyStates[0x3] = 1;
    }
    else
    {
        Chip8_KeyStates[0x3] = 0;
    }

    if (IsKeyDown(KEY_FOUR) || IsKeyPressed(KEY_FOUR))
    {
        Chip8_KeyStates[0xC] = 1;
    }
    else
    {
        Chip8_KeyStates[0xC] = 0;
    }

    if (IsKeyDown(KEY_Q) || IsKeyPressed(KEY_Q))
    {
        Chip8_KeyStates[0x4] = 1;
    }
    else
    {
        Chip8_KeyStates[0x4] = 0;
    }

    if (IsKeyDown(KEY_W) || IsKeyPressed(KEY_W))
    {
        Chip8_KeyStates[0x5] = 1;
    }
    else
    {
        Chip8_KeyStates[0x5] = 0;
    }

    if (IsKeyDown(KEY_E) || IsKeyPressed(KEY_E))
    {
        Chip8_KeyStates[0x6] = 1;
    }
    else
    {
        Chip8_KeyStates[0x6] = 0;
    }

    if (IsKeyDown(KEY_R) || IsKeyPressed(KEY_R))
    {
        Chip8_KeyStates[0xD] = 1;
    }
    else
    {
        Chip8_KeyStates[0xD] = 0;
    }

    if (IsKeyDown(KEY_A) || IsKeyPressed(KEY_A))
    {
        Chip8_KeyStates[0x7] = 1;
    }
    else
    {
        Chip8_KeyStates[0x7] = 0;
    }

    if (IsKeyDown(KEY_S) || IsKeyPressed(KEY_S))
    {
        Chip8_KeyStates[0x8] = 1;
    }
    else
    {
        Chip8_KeyStates[0x8] = 0;
    }

    if (IsKeyDown(KEY_D) || IsKeyPressed(KEY_D))
    {
        Chip8_KeyStates[0x9] = 1;
    }
    else
    {
        Chip8_KeyStates[0x9] = 0;
    }

    if (IsKeyDown(KEY_F) || IsKeyPressed(KEY_F))
    {
        Chip8_KeyStates[0xE] = 1;
    }
    else
    {
        Chip8_KeyStates[0xE] = 0;
    }

    if (IsKeyDown(KEY_Z) || IsKeyPressed(KEY_Z))
    {
        Chip8_KeyStates[0xA] = 1;
    }
    else
    {
        Chip8_KeyStates[0xA] = 0;
    }

    if (IsKeyDown(KEY_X) || IsKeyPressed(KEY_X))
    {
        Chip8_KeyStates[0x0] = 1;
    }
    else
    {
        Chip8_KeyStates[0x0] = 0;
    }

    if (IsKeyDown(KEY_C) || IsKeyPressed(KEY_C))
    {
        Chip8_KeyStates[0xB] = 1;
    }
    else
    {
        Chip8_KeyStates[0xB] = 0;
    }

    if (IsKeyDown(KEY_V) || IsKeyPressed(KEY_V))
    {
        Chip8_KeyStates[0xF] = 1;
    }
    else
    {
        Chip8_KeyStates[0xF] = 0;
    }
}

//------------------------------------------------------------------------------

void Chip8_ProcessDroppedFiles()
{
    // Has the user dropped a CHIP8 rom file on the Window?
    if (IsFileDropped())
    {
        int count = 0;
        char **dropfiles = GetDroppedFiles(&count);
        Chip8_Initialise();
        Chip8_LoadROM(dropfiles[0]);
        ClearDroppedFiles();
    }
    return;
}

//------------------------------------------------------------------------------

int fileexists(char *filename)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")))
    {
        fclose(fp);
        return 1;
    }
    return 0;
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    InitWindow(640, 320, "Chip8 Virtual Machine");
    char ROM_FileName[1024] = {'\0'};

    // Target Framerate
    SetTargetFPS(300);

    // Clear the client window
    BeginDrawing();
    ClearBackground(BLUE);
    //EndDrawing();

    // Initialise the Chip8 registers and memory.
    Chip8_Initialise();

    // Load the default logo rom.
    Chip8_LoadLogo();

    // No command line passed, so browse for a CHIP8 ROM file instead.
    if (argc == 1)
    {
        OpenFileDialog(ROM_FileName, sizeof(ROM_FileName));
        if (strlen(ROM_FileName) > 0)
        {
            Chip8_LoadROM(ROM_FileName);
        }
    }
    else if (argc == 2)
    {
        // Load the ROM file passed on the command line.
        if (fileexists((char *)argv[1]) == 1)
        {
            Chip8_LoadROM(argv[1]);
        }
    }

    Console_Show("Debug Window");

    // Initialise the delay timer.
    LastChip8_DelayUpdate = GetTime();
    while (!WindowShouldClose())
    {
        // Start Raylib drawing.
        BeginDrawing();

        // Clear the background.
        ClearBackground(BLACK);

        // Process any dropped chip8 files.
        Chip8_ProcessDroppedFiles();

        // Emulate one cpu cycle.
        Chip8_EmulateCPU();

        // Process the keypress states.
        Chip8_GetKeyStates();

        // Emulate one cpu cycle.
        Chip8_EmulateCPU();

        // Do we need to update the chip8 screen.
        Chip8_DrawGraphics();

        // Show chip 8 register status.
        Chip8_ShowProgramState();

        // End Raylib drawing.
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
