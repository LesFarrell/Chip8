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
#include <string.h>
#include <time.h>
#include "chip8.h"
#include "raylib.h"
#include "console.h"
#include "filedialogs.h"

const int CLIENTWIDTH = 640;
const int CLIENTHEIGHT = 320;

double LastChip8_DelayUpdate = 0;
double LastSoundUpdate = 0;

int store[16];

//------------------------------------------------------------------------------

/*
 * Function: Chip8_Initialise
 * TODO.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * void.
 */
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

    // Clear the register and key states.
    for (int i = 0; i < 16; ++i)
    {
        Chip8_VRegister[i] = 0;
        Chip8_KeyStates[i] = CHIP8_KEYUP;        
    }

    // Clear main program memory.
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

/*
 * Function: Chip8_DrawScreen
 * If the DrawFlag is set draws the CHIP screen
 *
 * Parameters:
 * None.
 *
 * Returns:
 * void.
 */
void Chip8_DrawScreen(void)
{
    int PixelWidth = CLIENTWIDTH / 64;
    int PixelHeight = CLIENTHEIGHT / 32;

    // If the draw flag is set then update the screen
    if (Chip8_DrawFlag == 1)
    {
        for (int row = 0; row < SCREENHEIGHT; ++row)
        {
            for (int col = 0; col < SCREENWIDTH; ++col)
            {
                if (Chip8_DisplayMemory[col + (row * SCREENWIDTH)] != 0)
                {
                    DrawRectangle(col * PixelWidth, row * PixelHeight, PixelWidth, PixelHeight, FOREGROUND);
                }
                else
                {
                    DrawRectangle(col * PixelWidth, row * PixelHeight, PixelWidth, PixelHeight, BACKGROUND);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------

/*
 * Function: Chip8_LoadROM
 * Loads the passed ROM file into program memory.
 *
 * Parameters:
 * ROM_FileName - Path to the ROM file.
 *
 * Returns:
 * int - EXIT_SUCCESS or EXIT_FAILURE.
 *
 */
int Chip8_LoadROM(char *ROM_FileName)
{
    FILE *fp;
    unsigned short pos = 512;
    unsigned char ch;

    fp = fopen(ROM_FileName, "rb");
    if (fp == NULL)
    {
        return EXIT_FAILURE;
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
    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------

/*
 * Function: Chip8_LoadDefaultROM
 * Simply loads a default ROM if nothing is loaded
 *
 * Parameters:
 * None.
 *
 * Returns:
 * (int ) - the returned value.
 *
 */
void Chip8_LoadDefaultROM(void)
{
    unsigned short pos = 512;
    for (int loop = 0; loop < sizeof(Chip8_LogoRom); loop++)
    {
        Chip8_ProgramMemory[pos++] = Chip8_LogoRom[loop];
    }
}

//------------------------------------------------------------------------------

/*
 * Function: Chip8_ShowProgramState
 * Shows the current state of the emulator Registers, Stack, Program Counter, etc.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * void.
 */
void Chip8_ShowProgramState(void)
{
    int i = 0;
    int y = 40;
    char string[100];

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
        sprintf(string, "V%02d : %02d\t", i, Chip8_VRegister[i]);
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

}

//------------------------------------------------------------------------------

/*
 * Function: Chip8_EmulateCPU
 * Emulates one cycle of the Chip8 CPU.
 *
 * Parameters:
 * void.
 *
 * Returns:
 * void.
 */
void Chip8_EmulateCPU(void)
{
    int keyPress = 0;
    double CurrentTime = GetTime();

    // Grab the next Chip8_OpCode.
    Chip8_OpCode = ((Chip8_ProgramMemory[Chip8_ProgramCounter] << 8) + Chip8_ProgramMemory[Chip8_ProgramCounter + 1]);

    // Extract the most common values from the OpCode
    int x = (Chip8_OpCode & 0x0F00) >> 8;
    int y = (Chip8_OpCode & 0x00F0) >> 4;
    int n = Chip8_OpCode & 0x000F;
    int kk = (Chip8_OpCode & 0x00FF);
    int nnn = (Chip8_OpCode & 0x0FFF);


    // Process the Chip8_OpCode.
    switch (Chip8_OpCode & 0xF000)
    {
        case 0x0000:
            switch (Chip8_OpCode & 0x00FF)
            {

            case 0x00C:
                printf("Scroll display down by %d\n",n);
                Chip8_ProgramCounter += 2;
                break;

            // 00E0 - CLS
            // Clear the display.
            case 0x00E0:
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
            case 0x00EE:
                Chip8_StackPointer--;
                Chip8_ProgramCounter = Chip8_Stack[Chip8_StackPointer];
                Chip8_ProgramCounter += 2;
                break;

            case 0x00FB:
                printf("Scroll Right 4 Pixels\n");
                Chip8_ProgramCounter += 2;
                break;

            case 0x00FC:
                printf("Scroll Left 4 Pixels\n");
                Chip8_ProgramCounter += 2;
                break;

            case 0x00FD:
                printf("Exit Chip Interpreter\n");
                Chip8_ProgramCounter += 2;
                break;

            case 0x00FE:
                printf("Disable Extended Screen Mode\n");
                Chip8_ProgramCounter += 2;
                break;

            case 0x00FF:
                printf("Enable Extended Screen Mode\n");
                Chip8_ProgramCounter += 2;
                break;
            }
            break;

        // 1nnn - JP addr
        // Jump to location nnn.
        // The interpreter sets the program counter to nnn.
        case 0x1000:
            Chip8_ProgramCounter = nnn;
            break;

        // 2nnn - CALL addr
        // Call subroutine at nnn.
        // The interpreter increments the stack pointer, then puts the Chip8_ProgramCounter value the top of the stack.
        // The Chip8_ProgramCounter is then set to nnn.
        case 0x2000:
            Chip8_Stack[Chip8_StackPointer] = Chip8_ProgramCounter;
            Chip8_StackPointer++;
            Chip8_ProgramCounter = nnn;
            break;

        // 3xkk - SE Vx, byte
        // Skip next instruction if Vx = kk.
        // The interpreter compares register Vx to kk, and if they are equal,
        // increments the program counter by 2.
        case 0x3000:
            if (Chip8_VRegister[x] == kk)
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
            if (Chip8_VRegister[x] != kk)
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
            if (Chip8_VRegister[x] == Chip8_VRegister[y])
            {
                Chip8_ProgramCounter += 2;
            }
            Chip8_ProgramCounter += 2;
            break;

        // 6xkk - LD Vx, byte
        // Set Vx = kk.
        // The interpreter puts the value kk into register Vx.
        case 0x6000:
            Chip8_VRegister[x] = kk;
            Chip8_ProgramCounter += 2;
            break;

        // 7xkk - ADD Vx, byte
        // Set Vx = Vx + kk.
        // Adds the value kk to the value of register Vx, then stores the result in Vx.
        case 0x7000:
            Chip8_VRegister[x] += kk;
            Chip8_ProgramCounter += 2;
            break;

        case 0x8000:
            switch (Chip8_OpCode & 0x000F)
            {

                // 8xy0 - LD Vx, Vy
                // Set Vx = Vy.
                // stores the value of register Vy in register Vx.
                case 0x000:
                    Chip8_VRegister[x] = Chip8_VRegister[y];
                    Chip8_ProgramCounter += 2;
                    break;

                // 8xy1 - OR Vx, Vy
                // Set Vx = Vx OR Vy.
                // Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
                // A bitwise OR compares the corresponding bits from two values,
                // and if either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0.
                case 0x001:
                    Chip8_VRegister[x] |= Chip8_VRegister[y];
                    Chip8_ProgramCounter += 2;
                    break;

                // 8xy2 - AND Vx, Vy
                // Set Vx = Vx AND Vy.
                // Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
                // A bitwise AND compares the corresponding bits from two values,
                // and if both bits are 1, then the same bit in the result is also 1. Otherwise, it is 0.
                case 0x002:
                    Chip8_VRegister[x] &= Chip8_VRegister[y];
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
                    Chip8_VRegister[x] ^= Chip8_VRegister[y];
                    Chip8_ProgramCounter += 2;
                    break;

                // 8xy4 - ADD Vx, Vy
                // Set Vx = Vx + Vy, set VF = carry.
                // The values of Vx and Vy are added together.
                // If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0.
                // Only the lowest 8 bits of the result are kept, and stored in Vx.
                case 0x0004:
                    if (Chip8_VRegister[y] > (255 - Chip8_VRegister[x]))
                    {
                        Chip8_VRegister[0xF] = 1;
                    }
                    else
                    {
                        Chip8_VRegister[0xF] = 0;
                    }
                    Chip8_VRegister[x] += Chip8_VRegister[y];
                    Chip8_ProgramCounter += 2;
                    break;

                // 8xy5 - SUB Vx, Vy
                // Set Vx = Vx - Vy, set VF = NOT borrow.
                // If Vx > Vy, then VF is set to 1, otherwise 0.
                // Then Vy is subtracted from Vx, and the results stored in Vx.
                case 0x0005:
                    printf("SUB V[%d]=%d, V[%d]=%d\n", x,Chip8_VRegister[x],y, Chip8_VRegister[y]);
                    if (Chip8_VRegister[x] >= Chip8_VRegister[y])
                    {
                        Chip8_VRegister[0xF] = 1;
                        printf("VF:SET\n", x, y);
                    }
                    else
                    {
                        Chip8_VRegister[0xF] = 0;
                        printf("VF:RESET\n", x, y);
                    }
                    Chip8_VRegister[x] -= Chip8_VRegister[y];
                    Chip8_ProgramCounter += 2;
                    break;

                // 8xy6 - SHR Vx {, Vy}
                // Set Vx = Vx SHR 1.
                // If the least-significant bit of Vx is 1, then VF is set to 1,
                // otherwise 0. Then Vx is divided by 2.
                case 0x0006:
                    Chip8_VRegister[0xF] = Chip8_VRegister[x] & 0x1;
                    Chip8_VRegister[x] = Chip8_VRegister[x] / 2;
                    Chip8_ProgramCounter += 2;
                    break;

                // 8xy7 - Subn Vx, Vy
                // Set Vx = Vy - Vx, set VF = NOT borrow.
                // If Vy > Vx, then VF is set to 1, otherwise 0.
                // Then Vx is subtracted from Vy, and the results stored in Vx.
                case 0x0007:
                    if (Chip8_VRegister[y] >= Chip8_VRegister[x])
                    {
                        Chip8_VRegister[0xF] = 1;
                    }
                    else
                    {
                        Chip8_VRegister[0xF] = 0;
                    }
                    Chip8_VRegister[x] = Chip8_VRegister[y] - Chip8_VRegister[x];
                    Chip8_ProgramCounter += 2;
                    break;

                // 8xyE - SHL Vx {, Vy}
                // Set Vx = Vx SHL 1.
                // If the most-significant bit of Vx is 1, then VF is set to 1,
                // otherwise to 0. Then Vx is multiplied by 2.
                case 0x000E:
                    Chip8_VRegister[0xF] = Chip8_VRegister[x] >> 7;
                    Chip8_VRegister[x] = Chip8_VRegister[x] * 2;
                    Chip8_ProgramCounter += 2;
                    break;
            }
            break;

        // 9xy0 - SNE Vx, Vy
        // Skip next instruction if Vx != Vy.
        // The values of Vx and Vy are compared and if they are not equal,
        // the program counter is increased by 2.
        case 0x9000:
            if (Chip8_VRegister[x] != Chip8_VRegister[y])
            {
                Chip8_ProgramCounter += 2;
            }
            Chip8_ProgramCounter += 2;
            break;

        // Annn - LD I, addr
        // Set Chip8_IndexRegister = nnn.
        // The value of register Chip8_IndexRegister is set to nnn.
        case 0xA000:
            Chip8_IndexRegister = nnn;
            Chip8_ProgramCounter += 2;
            break;

        // Bnnn - JP V0, addr
        // Jump to location nnn + V0.
        // The program counter is set to nnn plus the value of V0.
        case 0xB000:
            Chip8_ProgramCounter = nnn + Chip8_VRegister[0];
            break;

        // Cxkk - RND Vx, byte
        // Set Vx = random byte AND kk.
        // The interpreter generates a random number from 0 to 255,
        // which is then ANDed with the value kk. The results are stored in Vx.
        // See instruction 8xy2 for more information on AND.
        case 0xC000:
            Chip8_VRegister[x] = (rand() % 256) & kk;
            Chip8_ProgramCounter += 2;
            break;

        // Dxyn - DRW Vx, Vy, height
        // Display n-byte sprite starting at memory location Chip8_IndexRegister at (Vx, Vy), set VF = collision.
        // The interpreter reads n bytes from memory, starting at the address stored in I.
        // These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
        // Sprites are XORed onto the existing screen.
        // If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
        // If the sprite is positioned so part of it is outside the coordinates of the display,
        // it wraps around to the opposite side of the screen.
        case 0xD000:        

            Chip8_VRegister[0xF] = 0;

            for (int yline = 0; yline < n; yline++)
            {
                int bitvalue = Chip8_ProgramMemory[Chip8_IndexRegister + yline];
                for (int xline = 0; xline < 8; xline++)
                {
                    // Mask off each bit in the bit value.
                    if ((bitvalue & (0x80 >> xline)) != 0)
                    {

                        // Wrap the pixel coordinates using the % operator.
                        int col = (Chip8_VRegister[x] + xline) % SCREENWIDTH;
                        int row = (Chip8_VRegister[y] + yline) % SCREENHEIGHT;

                        // Calculate the screen memory address.
                        int address = col + (row * SCREENWIDTH);

                        // XOR and set flags as needed.
                        if (Chip8_DisplayMemory[address] == 1)
                        {
                            Chip8_VRegister[0xF] = 1;
                        }
                        Chip8_DisplayMemory[address] ^= 1;
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
                    if (Chip8_KeyStates[Chip8_VRegister[x]] == CHIP8_KEYDOWN)
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
                    if (Chip8_KeyStates[Chip8_VRegister[x]] == CHIP8_KEYUP)
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
                    Chip8_VRegister[x] = Chip8_DelayTimer;
                    Chip8_ProgramCounter += 2;
                    break;

                // Fx0A - LD Vx, K
                // Wait for a key press, store the value of the key in Vx.
                // All execution stops until a key is pressed,
                // then the value of that key is stored in Vx.
                case 0x000A:
                    
                    for (int i = 0; i < 16; i++)
                    {
                        if (Chip8_KeyStates[i] == CHIP8_KEYDOWN)
                        {
                            Chip8_VRegister[x] = i;
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
                    Chip8_DelayTimer = Chip8_VRegister[x];
                    Chip8_ProgramCounter += 2;
                    break;

                // Fx18 - LD Chip8_SoundTimer, Vx
                // Set sound timer = Vx.
                // Chip8_SoundTimer is set equal to the value of Vx.
                case 0x0018:
                    Chip8_SoundTimer = Chip8_VRegister[x];
                    Chip8_ProgramCounter += 2;
                    break;

                // Fx1E - ADD I, Vx
                // Set Chip8_IndexRegister = Chip8_IndexRegister + Vx.
                // The values of I and Vx are added, and the results are stored in I.
                case 0x001E:
                    // VF is set to 1 when range overflow (Chip8_IndexRegister + VX > 0xFFF), and 0 when it isn't.
                    if (Chip8_IndexRegister + Chip8_VRegister[x] > 0xFFF)
                    {
                        Chip8_VRegister[0xF] = 1;
                    }
                    else
                    {
                        Chip8_VRegister[0xF] = 0;
                    }
                    Chip8_IndexRegister += Chip8_VRegister[x];
                    Chip8_ProgramCounter += 2;
                    break;

                // Fx29 - LD F, Vx
                // Set Chip8_IndexRegister = location of sprite for digit Vx.
                // The value of Chip8_IndexRegister is set to the location for the hexadecimal sprite corresponding to the value of Vx.
                case 0x0029:
                    Chip8_IndexRegister = Chip8_VRegister[x] * 0x5;
                    Chip8_ProgramCounter += 2;
                    break;


                case 0x0030:
                    printf("Point I to 10-byte font Sprite for digit VX\n");
                    Chip8_ProgramCounter += 2;
                    break;

                // Fx33 - LD B, Vx
                // store BCD representation of Vx in memory locations Chip8_IndexRegister, Chip8_IndexRegister+1, and Chip8_IndexRegister+2.
                // The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in Chip8_IndexRegister,
                // the tens digit at location I + 1, and the ones digit at location Chip8_IndexRegister + 2.
                case 0x0033:
                    Chip8_ProgramMemory[Chip8_IndexRegister] = Chip8_VRegister[x] / 100;
                    Chip8_ProgramMemory[Chip8_IndexRegister + 1] = (Chip8_VRegister[x] / 10) % 10;
                    Chip8_ProgramMemory[Chip8_IndexRegister + 2] = (Chip8_VRegister[x] % 100) % 10;
                    Chip8_ProgramCounter += 2;
                    break;

                // Fx55 - LD [Chip8_IndexRegister], Vx
                // store registers V0 through Vx in memory starting at location Chip8_IndexRegister.
                // The interpreter copies the values of registers V0 through Vx into memory, starting at the address in Chip8_IndexRegister.
                case 0x0055:
                    for (int i = 0; i <= x; i++)
                    {
                        Chip8_ProgramMemory[Chip8_IndexRegister + i] = Chip8_VRegister[i];
                    }
                    Chip8_ProgramCounter += 2;
                    break;

                // Fx65 - LD Vx, [I]
                // Read registers V0 through Vx from memory starting at location I.
                // The interpreter reads values from memory starting at location I into registers V0 through Vx.
                case 0x0065:
                    for (int i = 0; i <= x; i++)
                    {
                        Chip8_VRegister[i] = Chip8_ProgramMemory[Chip8_IndexRegister + i];
                    }
                    Chip8_ProgramCounter += 2;
                    break;
                    
                case 0x0075:
                    printf("Store V0..VX in RPL user flags (X <= 7)\n");
                    
                    for (int c=0; c <= x; c++)
                    {
                        printf(" store[%d]= Chip8_VRegister[%d] \n", store[c], Chip8_VRegister[c] );
                        store[c]=Chip8_VRegister[c];
                    }
                    
                    Chip8_ProgramCounter += 2;
                    break;

                case 0x0085:
                    printf("Read V0..VX in RPL user flags (X <= 7)\n");
                    for (int c = 0; c <= x; c++)
                    {
                        printf("Chip8_VRegister[%d] = store[%d]\n", Chip8_VRegister[c], store[c]);
                            Chip8_VRegister[c] = store[c];
                    }

                    printf("Read V0..VX in RPL user flags (X <= 7)\n");
                    Chip8_ProgramCounter += 2;
                    break;
                        
            }
            break;
    }


    // Is it time to decrement the Chip8_DelayTimer. (0.016666 = one second / 60 = 60hz)
    if (CurrentTime - LastChip8_DelayUpdate > 0.01666)
    {
        if (Chip8_DelayTimer > 0)
        {
            Chip8_DelayTimer--;
        }
        LastChip8_DelayUpdate = CurrentTime;
    }


    // Is it time to decrement the Chip8_SoundTimer. (0.01666 = one second / 60 = 60hz)
    // No sound implemented at the moment, enjoy the silence!
    if (CurrentTime - LastSoundUpdate > 0.01666)
    {
        if (Chip8_SoundTimer > 0)
        {
            Chip8_SoundTimer = Chip8_SoundTimer - 1;
        }
        LastSoundUpdate = CurrentTime;
    }
}

//------------------------------------------------------------------------------

/*
 * Function: Chip8_GetKeyStates
 * Processes and stores the Chip8 keyboard state.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * void.
 */
void Chip8_GetKeyStates(void)
{
    if (IsKeyDown(KEY_ONE) || IsKeyPressed(KEY_ONE))
    {
        Chip8_KeyStates[0x1] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0x1] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_TWO) || IsKeyPressed(KEY_TWO))
    {
        Chip8_KeyStates[0x2] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0x2] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_THREE) || IsKeyPressed(KEY_THREE))
    {
        Chip8_KeyStates[0x3] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0x3] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_FOUR) || IsKeyPressed(KEY_FOUR))
    {
        Chip8_KeyStates[0xC] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0xC] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_Q) || IsKeyPressed(KEY_Q))
    {
        Chip8_KeyStates[0x4] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0x4] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_W) || IsKeyPressed(KEY_W))
    {
        Chip8_KeyStates[0x5] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0x5] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_E) || IsKeyPressed(KEY_E))
    {
        Chip8_KeyStates[0x6] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0x6] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_R) || IsKeyPressed(KEY_R))
    {
        Chip8_KeyStates[0xD] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0xD] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_A) || IsKeyPressed(KEY_A))
    {
        Chip8_KeyStates[0x7] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0x7] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_S) || IsKeyPressed(KEY_S))
    {
        Chip8_KeyStates[0x8] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0x8] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_D) || IsKeyPressed(KEY_D))
    {
        Chip8_KeyStates[0x9] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0x9] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_F) || IsKeyPressed(KEY_F))
    {
        Chip8_KeyStates[0xE] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0xE] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_Z) || IsKeyPressed(KEY_Z))
    {
        Chip8_KeyStates[0xA] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0xA] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_X) || IsKeyPressed(KEY_X))
    {
        Chip8_KeyStates[0x0] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0x0] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_C) || IsKeyPressed(KEY_C))
    {
        Chip8_KeyStates[0xB] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0xB] = CHIP8_KEYUP;
    }

    if (IsKeyDown(KEY_V) || IsKeyPressed(KEY_V))
    {
        Chip8_KeyStates[0xF] = CHIP8_KEYDOWN;
    }
    else
    {
        Chip8_KeyStates[0xF] = CHIP8_KEYUP;
    }
}

//------------------------------------------------------------------------------

/*
 * Function: Chip8_ProcessDroppedFiles
 * If the user dropped a ROM file on the application then initialise Chip8 and load it.
 *
 * Parameters:
 * None.
 *
 * Returns:
 * void.
 */
void Chip8_ProcessDroppedFiles(void)
{
    // Has the user dropped a CHIP8 ROM file onto the Window?
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

/*
 * Function: main
 * Main entry point for the application.
 *
 * Parameters:
 * int argc     - Number of command line parameters
 * char *argv[] - Array of the the command line parameters
 *
 * Returns:
 * int.
 */
int main(int argc, char *argv[])
{
    char ROM_FileName[1024] = {'\0'};

    // Initialise the applications window
    InitWindow(CLIENTWIDTH, CLIENTHEIGHT, "Chip8 - Virtual Machine");

    // Set the Target Framerate
    SetTargetFPS(150);

    // Clear the client window contents before we start.
    BeginDrawing();
    ClearBackground(BACKGROUND);
    EndDrawing();

    // Initialise the Chip8 registers
    Chip8_Initialise();

    // Load the default logo rom
    Chip8_LoadDefaultROM();

    // No command line passed, so browse for a CHIP8 ROM file instead.
    if (argc == 1)
    {
        OpenFileDialog(ROM_FileName, sizeof(ROM_FileName));
        if (strlen(ROM_FileName) > 0)
        {
            // Load the selected rom file.
            Chip8_LoadROM(ROM_FileName);
        }
    }

    // Load the ROM file passed on the command line
    else if (argc == 2)
    {
        Chip8_LoadROM(argv[1]);
    }

    // Console_Show("Debug Window");

    // Store the current time, it's used for the delay timers.
    LastChip8_DelayUpdate = GetTime();

    // Loop until the user exits.
    while (!WindowShouldClose())
    {
        // Start Raylib drawing.
        BeginDrawing();

        // Clear the background.
        ClearBackground(BACKGROUND);

        // Process any dropped chip8 files.
        Chip8_ProcessDroppedFiles();

        // Process the keypress states.
        Chip8_GetKeyStates();

        // Emulate one cpu cycle.
        Chip8_EmulateCPU();

        // Update the chip8 screen if needed.
        Chip8_DrawScreen();

        // Show chip 8 register status.
        // Chip8_ShowProgramState();

        // End Raylib drawing.
        EndDrawing();
    }

    // Close the window and shut down Raylib.
    CloseWindow();

    // Return to the OS.
    return EXIT_SUCCESS;
}
