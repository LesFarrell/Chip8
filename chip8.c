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
#include "console.h"
#include "filedialogs.h"

const int CLIENTWIDTH = 640;
const int CLIENTHEIGHT = 320;

const int CHIP8TICKSPERFRAME = 10;

double Last_DelayUpdate = 0;
double LastSoundUpdate = 0;
double CurrentTime = 0;

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
 * Function: Chip8_Initialise
 * Sets the intial states for the emulator.
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
  CHIP8_SUPER = 0;              // Default to a Standard Chip8

  CHIP8_SCREENWIDTH = 64;
  CHIP8_SCREENHEIGHT = 32;

  // Clear the display memory.
  for (int i = 0; i < 8192; ++i)
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

  // Load the default Chip-8 font into memory.
  int count = 0;
  for (int i = 0; i < 80; ++i)
  {
    Chip8_ProgramMemory[i] = Chip8_FontSet[count++];
  }

  // Load the Super Chip extended font into memory.
  count = 0;
  for (int i = 80; i < 240; ++i)
  {
    Chip8_ProgramMemory[i] = Chip8_SuperFontSet[count++];
  }

  // Load our default splash ROM
  unsigned short pos = 512;
  for (int loop = 0; loop < sizeof(Chip8_LogoRom); loop++)
  {
    Chip8_ProgramMemory[pos++] = Chip8_LogoRom[loop];
  }

  // Initialise the random seed.
  srand(time(NULL));
}

//------------------------------------------------------------------------------

/*
 * Function: Chip8_DrawScreen
 * If the DrawFlag is set draws the CHIP screen.
 *
 * Parameters:
 * Tigr *screen.
 *
 * Returns:
 * void.
 */
void Chip8_DrawScreen(Tigr *screen)
{
  int PixelWidth = CLIENTWIDTH / CHIP8_SCREENWIDTH;
  int PixelHeight = CLIENTHEIGHT / CHIP8_SCREENHEIGHT;

  // If the draw flag is set then update the screen
  if (Chip8_DrawFlag == 1)
  {
    for (int row = 0; row < CHIP8_SCREENHEIGHT; ++row)
    {
      for (int col = 0; col < CHIP8_SCREENWIDTH; ++col)
      {
        if (Chip8_DisplayMemory[col + (row * CHIP8_SCREENWIDTH)] != 0)
        {
          tigrFill(screen, col * PixelWidth, row * PixelHeight, PixelWidth, PixelHeight, FOREGROUND);
        }
        else
        {
          tigrFill(screen, col * PixelWidth, row * PixelHeight, PixelWidth, PixelHeight, BACKGROUND);
        }
      }
    }
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
  Console_TextXY(string, 20, 2);

  sprintf(string, "Sound : %d\t", Chip8_SoundTimer);
  Console_TextXY(string, 20, 3);

  y = 5;
  for (i = 0; i < 16; i++)
  {
    sprintf(string, "V[%02d] : %02d\t", i, Chip8_VRegister[i]);
    Console_TextXY(string, 1, y);
    y = y + 1;
  }

  y = 5;
  for (i = 0; i < 16; i++)
  {
    sprintf(string, "Stack[%02d] : %02d\t", i, Chip8_Stack[i]);
    Console_TextXY(string, 20, y);
    y = y + 1;
  }

  y = 5;
  for (i = 0; i < 16; i++)
  {
    sprintf(string, "Key [%02d] : %02d\t", i, Chip8_KeyStates[i]);
    Console_TextXY(string, 40, y + i);
  }
}

//------------------------------------------------------------------------------

void Chip8_Disassemble(void)
{
  char string[100];
  char OpCode[100];

  // Grab the next Chip8_OpCode.
  Chip8_OpCode = ((Chip8_ProgramMemory[Chip8_ProgramCounter] << 8) + Chip8_ProgramMemory[Chip8_ProgramCounter + 1]);

  sprintf(OpCode, "%04X: %04X - [%3d, %3d]  : ", Chip8_ProgramCounter, Chip8_OpCode, Chip8_ProgramMemory[Chip8_ProgramCounter], Chip8_ProgramMemory[Chip8_ProgramCounter + 1]);

  // Extract the most common values from the OpCode
  int x = (Chip8_OpCode & 0x0F00) >> 8;
  int y = (Chip8_OpCode & 0x00F0) >> 4;
  int n = (Chip8_OpCode & 0x000F);
  int kk = (Chip8_OpCode & 0x00FF);
  int nnn = (Chip8_OpCode & 0x0FFF);

  // Process the Chip8_OpCode.
  switch (Chip8_OpCode & 0xF000)
  {

  case 0x0000:
    switch (Chip8_OpCode & 0x00F0)
    {

    // 000CN - Scroll down
    case 0x0C0:
      sprintf(string, "00C0 - Scroll Down %d\n", n);
      break;
    }

    switch (Chip8_OpCode & 0x00FF)
    {

    // 00E0 - CLS
    case 0x00E0:
      sprintf(string, "00E0 - CLS\n");
      break;

    // 00EE - RET
    case 0x00EE:
      sprintf(string, "00EE - RET\n");
      break;

    // 00FB - Scroll Right 4 Pixels.
    case 0x00FB:
      sprintf(string, "00FB - Scroll Right\n");
      break;

    // 00FC - Scroll Left 4 Pixels.
    case 0x00FC:
      sprintf(string, "00FC - Scroll Left\n");
      break;

    // 0x00FD - Exit the Chip8 Interpreter.
    case 0x00FD:
      sprintf(string, "0x00FD - Exit Chip8\n");
      break;

    // 0x00FE - Disable Super Chip Mode.
    case 0x00FE:
      sprintf(string, "0x00FE - Disable Super Chip\n");
      break;

    // 0x00FF - Enable Super Chip Mode.
    case 0x00FF:
      sprintf(string, "0x00FF - Enable Super Chip\n");
      break;

    default:
      break;
    }
    break;

  // 1NNN - JP addr
  case 0x1000:
    sprintf(string, "1NNN - JP %04X\n", nnn);
    break;

  // 2NNN - CALL addr
  case 0x2000:
    sprintf(string, "2NNN - CALL %d\n", nnn);
    break;

  // 3XKK - SE Vx, byte
  case 0x3000:
    sprintf(string, "3XKK - SE V[%d], %d\n", x, kk);
    break;

  // 4XKK - SNE Vx, byte
  case 0x4000:
    sprintf(string, "4XKK - SNE V[%d], %d\n", x, kk);
    break;

  // 5XY0 - SE Vx, Vy
  case 0x5000:
    sprintf(string, "5XY0 - SE V[%d], V[%d]\n", x, y);
    break;

  // 6XKK - LD Vx, byte
  case 0x6000:
    sprintf(string, "6XKK - LD V[%d], %d\n", x, kk);
    break;

  // 7XKK - ADD Vx, byte
  case 0x7000:
    sprintf(string, "7XKK - ADD V[%d], %d\n", x, kk);
    break;

  case 0x8000:
    switch (Chip8_OpCode & 0x000F)
    {

    // 8XY0 - LD Vx, Vy
    case 0x000:
      sprintf(string, "8XY0 - LD V[%d], V[%d]\n", x, y);
      break;

    // 8XY1 - OR Vx, Vy
    case 0x001:
      sprintf(string, "8XY1 - OR V[%d], V[%d]\n", x, y);
      break;

    // 8XY2 - AND Vx, Vy
    case 0x002:
      sprintf(string, "8XY2 - AND V[%d], V[%d]\n", x, y);
      break;

    // 8XY3 - XOR Vx, Vy
    case 0x003:
      sprintf(string, "8XY3 - XOR V[%d], V[%d]\n", x, y);
      break;

    // 8XY4 - ADD Vx, Vy
    case 0x0004:
      sprintf(string, "8XY4 - ADD V[%d], V[%d]\n", x, y);
      break;

    // 8XY5 - SUB Vx, Vy
    case 0x0005:
      sprintf(string, "8XY5 - SUB V[%d], V[%d]\n", x, y);
      break;

    // 8XY6 - SHR Vx {, Vy}
    case 0x0006:
      sprintf(string, "8XY6 - SHR V[%d] {, V[%d]}\n", x, y);
      break;

    // 8XY7 - Subn Vx, Vy
    case 0x0007:
      sprintf(string, "8XY7 - SUBN V[%d], V[%d]\n", x, y);
      break;

    // 8XYE - SHL Vx {, Vy}
    case 0x000E:
      sprintf(string, "8XYE - SHL V[%d] {, V[%d]}\n", x, y);
      break;

    default:
      sprintf(string, "Unknown Op Code: %04X\n", Chip8_OpCode);
      break;
    }
    break;

  // 9XY0 - SNE Vx, Vy
  case 0x9000:
    sprintf(string, "9XY0 - SNE V[%d], V[%d]\n", x, y);
    break;

  // ANNN - LD I, addr
  case 0xA000:
    sprintf(string, "ANNN - LD I, %d\n", nnn);
    break;

  // BNNN - JP V0, addr
  case 0xB000:
    sprintf(string, "BNNN - JP V0, %04X\n", nnn);
    break;

  // CXKK - RND Vx, byte
  case 0xC000:
    sprintf(string, "CXKK - RND V[%d], %d\n", x, kk);
    break;

  // DXYN - DRW Vx, Vy, height
  case 0xD000:
    sprintf(string, "DXYN - DRW V[%d], V[%d], %d\n", x, y, n);
    break;

  case 0xE000:
    switch (Chip8_OpCode & 0x00FF)
    {
    // EX9E - SKP Vx
    case 0x009E:
      sprintf(string, "EX9E - SKP V[%d]\n", x);
      break;

    // EXA1 - SKNP Vx
    case 0x00A1:
      sprintf(string, "EXA1 - SKNP V[%d]\n", x);
      break;

    default:
      sprintf(string, "Unknown Op Code: %04X\n", Chip8_OpCode);
      break;
    }
    break;

  case 0xF000:
    switch (Chip8_OpCode & 0x00FF)
    {

    // Fx07 - LD Vx, Chip8_DelayTimer
    case 0x0007:
      sprintf(string, "FX07 - LD V[%d], Delay\n", x);
      break;

    // Fx0A - LD Vx, K
    case 0x000A:
      sprintf(string, "FX0A - LD V[%d], K\n", x);
      break;

    // Fx15 - LD Chip8_DelayTimer, Vx
    case 0x0015:
      sprintf(string, "FX15 - LD Delay, V[%d]\n", x);
      break;

    // Fx18 - LD Chip8_SoundTimer, Vx
    case 0x0018:
      sprintf(string, "FX18 - LD Sound, V[%d]\n", x);
      break;

    // FX1E - ADD I, Vx
    case 0x001E:
      sprintf(string, "FX1E - ADD I, V[%d]\n", x);
      break;

    // FX29 - LD F, Vx
    case 0x0029:
      sprintf(string, "FX29 - LD F, V[%d]\n", x);
      break;

    // FX30 - Point I to hires font Vx
    case 0x0030:
      sprintf(string, "FX30 - Point I to font V[%d]\n", x);
      break;

    // FX33 - LD B, Vx
    case 0x0033:
      sprintf(string, "FX33 - LD B, V[%d]\n", x);
      break;

    // FX55 - LD [Chip8_IndexRegister], Vx
    case 0x0055:
      sprintf(string, "FX55 - LD I, V[%d]\n", x);
      break;

    // FX65 - LD Vx, [I]
    case 0x0065:
      sprintf(string, "FX65 - LD V[%d], I\n", x);
      break;

    // FX75 - Store V0..VX
    case 0x0075:
      sprintf(string, "FX75 - Store V0..VX\n");
      break;

    // FX85 - Read V0..VX
    case 0x0085:
      sprintf(string, "FX85 - Read V0..VX\n");
      break;

    default:
      sprintf(string, "Unknown Op Code: %04X\n", Chip8_OpCode);
      break;
    }
    break;
  }

  Console_TextXY("                                                                                                                                  ", 1, 22);
  Console_TextXY(OpCode, 1, 22);
  Console_TextXY(string, 30, 22);
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

  unsigned char Destination[8192] = {0};

  // Grab the next Chip8_OpCode.
  Chip8_OpCode = ((Chip8_ProgramMemory[Chip8_ProgramCounter] << 8) + Chip8_ProgramMemory[Chip8_ProgramCounter + 1]);

  // Extract the most common values from the OpCode
  int x = (Chip8_OpCode & 0x0F00) >> 8;
  int y = (Chip8_OpCode & 0x00F0) >> 4;
  int n = (Chip8_OpCode & 0x000F);
  int kk = (Chip8_OpCode & 0x00FF);
  int nnn = (Chip8_OpCode & 0x0FFF);

  // Process the Chip8_OpCode.
  switch (Chip8_OpCode & 0xF000)
  {

  case 0x0000:
    switch (Chip8_OpCode & 0x00F0)
    {
    // 000C - Scroll Down n lines
    case 0x0C0:
      for (int loop = 0; loop < 8192; loop++)
      {
        Destination[loop] = 0;
      }
      for (int col = 0; col < CHIP8_SCREENWIDTH; col++)
      {
        for (int row = 0; row < CHIP8_SCREENHEIGHT - n; row++)
        {
          int Source = col + (row * (CHIP8_SCREENWIDTH));
          int Dest = col + ((row + n) * (CHIP8_SCREENWIDTH));
          Destination[Dest] = Chip8_DisplayMemory[Source];
        }
      }
      for (int loop = 0; loop < 8192; loop++)
      {
        Chip8_DisplayMemory[loop] = Destination[loop];
      }
      Chip8_ProgramCounter += 2;
      break;
    }

    switch (Chip8_OpCode & 0x00FF)
    {

    // 00E0 - CLS
    case 0x00E0:
      // Clear the display.
      for (int c = 0; c < 8192; c++)
      {
        Chip8_DisplayMemory[c] = 0;
      }
      Chip8_DrawFlag = 1;
      Chip8_ProgramCounter += 2;
      break;

    // 00EE - RET Return from a subroutine.
    case 0x00EE:
      // Sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
      Chip8_ProgramCounter = Chip8_Stack[--Chip8_StackPointer];
      // Chip8_StackPointer--;
      Chip8_ProgramCounter += 2;
      break;

    // 00FB - Scroll Right 4 Pixels.
    case 0x00FB:
      for (int loop = 0; loop < 8192; loop++)
      {
        Destination[loop] = 0;
      }
      for (int col = 0; col < CHIP8_SCREENWIDTH - 4; col++)
      {
        for (int row = 0; row < CHIP8_SCREENHEIGHT; row++)
        {
          int Source = col + (row * (CHIP8_SCREENWIDTH));
          int Dest = (col + 4) + (row * (CHIP8_SCREENWIDTH));
          Destination[Dest] = Chip8_DisplayMemory[Source];
        }
      }
      for (int loop = 0; loop < 8192; loop++)
      {
        Chip8_DisplayMemory[loop] = Destination[loop];
      }
      Chip8_DrawFlag = 1;
      Chip8_ProgramCounter += 2;
      break;

    // 00FC - Scroll Left 4 Pixels.
    case 0x00FC:
      for (int loop = 0; loop < 8192; loop++)
      {
        Destination[loop] = 0;
      }
      for (int col = 4; col < CHIP8_SCREENWIDTH; col++)
      {
        for (int row = 0; row < CHIP8_SCREENHEIGHT; row++)
        {
          int Source = col + (row * (CHIP8_SCREENWIDTH));
          int Dest = (col - 4) + (row * (CHIP8_SCREENWIDTH));
          Destination[Dest] = Chip8_DisplayMemory[Source];
        }
      }
      for (int loop = 0; loop < 8192; loop++)
      {
        Chip8_DisplayMemory[loop] = Destination[loop];
      }
      Chip8_DrawFlag = 1;
      Chip8_ProgramCounter += 2;
      break;

    // 0x00FD - Exit the Chip8 Interpreter
    case 0x00FD:
      Chip8_Initialise();
      break;

    // 0x00FE - Disable Super Chip Mode
    case 0x00FE:
      CHIP8_SUPER = 0;
      CHIP8_SCREENWIDTH = 64;
      CHIP8_SCREENHEIGHT = 32;
      Chip8_ProgramCounter += 2;
      break;

    // 0x00FF - Enable Super Chip Mode
    case 0x00FF:
      CHIP8_SUPER = 1;
      CHIP8_SCREENWIDTH = 128;
      CHIP8_SCREENHEIGHT = 64;
      Chip8_ProgramCounter += 2;
      break;

    default:
      break;
    }
    break;

  // 1NNN - JP nnn Jump to location nnn.
  case 0x1000:
    // The interpreter sets the program counter to nnn.
    Chip8_ProgramCounter = nnn;
    break;

  // 2NNN - CALL addr
  case 0x2000:
    // Put the Chip8_ProgramCounter value the top of the stack.
    Chip8_Stack[Chip8_StackPointer] = Chip8_ProgramCounter;

    // The interpreter increments the stack pointer
    Chip8_StackPointer++;

    // The Chip8_ProgramCounter is then set to nnn.
    Chip8_ProgramCounter = nnn;
    break;

  // 3XKK - SE Vx, kk
  case 0x3000:
    // The interpreter compares register Vx to kk
    if (Chip8_VRegister[x] == kk)
    {
      // if they are equal increments the program counter by 2.
      Chip8_ProgramCounter += 2;
    }
    Chip8_ProgramCounter += 2;
    break;

  // 4XKK - SNE Vx, byte
  case 0x4000:
    // Skip next instruction if Vx != kk.
    if (Chip8_VRegister[x] != kk)
    {
      // increments the program counter by 2.
      Chip8_ProgramCounter += 2;
    }
    Chip8_ProgramCounter += 2;
    break;

  // 5XY0 - SE Vx, Vy
  case 0x5000:
    // The interpreter compares register Vx to register Vy
    if (Chip8_VRegister[x] == Chip8_VRegister[y])
    {
      // if they are equal, increments the program counter by 2.
      Chip8_ProgramCounter += 2;
    }
    Chip8_ProgramCounter += 2;
    break;

  // 6XKK - LD Vx, kk
  case 0x6000:
    Chip8_VRegister[x] = kk;
    Chip8_ProgramCounter += 2;
    break;

  // 7XKK - ADD Vx, kk
  case 0x7000:
    Chip8_VRegister[x] += kk;
    Chip8_ProgramCounter += 2;
    break;

  case 0x8000:
    switch (Chip8_OpCode & 0x000F)
    {

    // 8XY0 - LD Vx, Vy
    case 0x000:
      Chip8_VRegister[x] = Chip8_VRegister[y];
      Chip8_ProgramCounter += 2;
      break;

    // 8XY1 - OR Vx, Vy
    case 0x001:
      // Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
      Chip8_VRegister[x] |= Chip8_VRegister[y];
      Chip8_ProgramCounter += 2;
      break;

    // 8XY2 - AND Vx, Vy
    case 0x002:
      // Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
      Chip8_VRegister[x] &= Chip8_VRegister[y];
      Chip8_ProgramCounter += 2;
      break;

    // 8XY3 - XOR Vx, Vy
    case 0x003:
      // Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
      Chip8_VRegister[x] ^= Chip8_VRegister[y];
      Chip8_ProgramCounter += 2;
      break;

    // 8XY4 - ADD Vx, Vy
    case 0x0004:
      // The values of Vx and Vy are added together.
      // If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0.
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

    // 8XY5 - SUB Vx, Vy
    case 0x0005:
      // If Vx > Vy, then VF is set to 1, otherwise 0.
      // Chip8_VRegister[x] = Chip8_VRegister[x] - Chip8_VRegister[y];

      if (Chip8_VRegister[x] >= Chip8_VRegister[y])
      {
        Chip8_VRegister[0xF] = 1;
      }
      else
      {
        Chip8_VRegister[0xF] = 0;
      }

      // Then Vy is subtracted from Vx, and the results stored in Vx.
      Chip8_VRegister[x] -= Chip8_VRegister[y];
      Chip8_ProgramCounter += 2;
      break;

    // 8XY6 - SHR Vx {, Vy}
    case 0x0006:
      // If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
      Chip8_VRegister[0xF] = Chip8_VRegister[x] & 0x1;
      Chip8_VRegister[x] = Chip8_VRegister[x] / 2;
      Chip8_ProgramCounter += 2;
      break;

    // 8XY7 - Subn Vx, Vy
    case 0x0007:
      // If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
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

    // 8XYE - SHL Vx {, Vy}
    case 0x000E:
      // If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
      Chip8_VRegister[0xF] = Chip8_VRegister[x] >> 7;
      Chip8_VRegister[x] = Chip8_VRegister[x] * 2;
      Chip8_ProgramCounter += 2;
      break;

    default:
      break;
    }
    break;

  // 9XY0 - SNE Vx, Vy
  case 0x9000:
    // The values of Vx and Vy are compared and if they are not equal, the program counter is increased by 2.
    if (Chip8_VRegister[x] != Chip8_VRegister[y])
    {
      Chip8_ProgramCounter += 2;
    }
    Chip8_ProgramCounter += 2;
    break;

  // ANNN - LD I, addr
  case 0xA000:
    // Set Chip8_IndexRegister = nnn.
    Chip8_IndexRegister = nnn;
    Chip8_ProgramCounter += 2;
    break;

  // BNNN - JP V0, addr
  case 0xB000:
    // Jump to location nnn + V0.
    Chip8_ProgramCounter = nnn + Chip8_VRegister[0];
    break;

  // CXKK - RND Vx, byte
  case 0xC000:
    // Set Vx = random byte AND kk.
    Chip8_VRegister[x] = (rand() % 256) & kk;
    Chip8_ProgramCounter += 2;
    break;

  // DXYn - DRW Vx, Vy, height
  // Display n-byte sprite starting at memory location Chip8_IndexRegister at (Vx, Vy), set VF = collision.
  // The interpreter reads n bytes from memory, starting at the address stored in I.
  // These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
  // Sprites are XORed onto the existing screen.
  // If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
  // If the sprite is positioned so part of it is outside the coordinates of the display,
  // it wraps around to the opposite side of the screen.
  case 0xD000:
    Chip8_VRegister[0xF] = 0;

    if (CHIP8_SUPER == 0)
    {
      for (int yline = 0; yline < n; yline++)
      {
        int bitvalue = Chip8_ProgramMemory[Chip8_IndexRegister + yline];
        for (int xline = 0; xline < 8; xline++)
        {
          // Mask off each bit in the bit value.
          if ((bitvalue & (0x80 >> xline)) != 0)
          {
            // Wrap the pixel coordinates using the % operator.
            int col = (Chip8_VRegister[x] + xline) % CHIP8_SCREENWIDTH;
            int row = (Chip8_VRegister[y] + yline) % CHIP8_SCREENHEIGHT;

            // Calculate the screen memory address.
            int address = col + (row * CHIP8_SCREENWIDTH);

            // XOR and set flags as needed.
            if (Chip8_DisplayMemory[address] == 1)
            {
              Chip8_VRegister[0xF] = 1;
            }
            Chip8_DisplayMemory[address] ^= 1;
          }
        }
      }
    }
    else
    {
      if (n == 0)
      {
        // Draw 16x16 sprite
        int offset = 0;
        for (int yline = 0; yline < 16; yline++)
        {
          unsigned int bitvalue = (Chip8_ProgramMemory[Chip8_IndexRegister + offset] * 256) + (Chip8_ProgramMemory[Chip8_IndexRegister + (offset + 1)]);
          offset += 2;

          for (int xline = 0; xline < 16; xline++)
          {
            // Mask off each bit in the bit value.
            if ((bitvalue & (0x8000 >> xline)) != 0)
            {
              // Wrap the pixel coordinates using the % operator.
              int col = (Chip8_VRegister[x] + xline) % CHIP8_SCREENWIDTH;
              int row = (Chip8_VRegister[y] + yline) % CHIP8_SCREENHEIGHT;

              // Calculate the screen memory address.
              int address = col + (row * CHIP8_SCREENWIDTH);

              // XOR and set flags as needed.
              if (Chip8_DisplayMemory[address] == 1)
              {
                Chip8_VRegister[0xF] = 1;
              }
              Chip8_DisplayMemory[address] ^= 1;
            }
          }
        }
      }
      else
      {
        // Draw 8xN graphic
        for (int yline = 0; yline < n; yline++)
        {
          int bitvalue = Chip8_ProgramMemory[Chip8_IndexRegister + yline];
          for (int xline = 0; xline < 8; xline++)
          {
            // Mask off each bit in the bit value.
            if ((bitvalue & (0x80 >> xline)) != 0)
            {

              // Wrap the pixel coordinates using the % operator.
              int col = (Chip8_VRegister[x] + xline) % CHIP8_SCREENWIDTH;
              int row = (Chip8_VRegister[y] + yline) % CHIP8_SCREENHEIGHT;

              // Calculate the screen memory address.
              int address = col + (row * CHIP8_SCREENWIDTH);

              // XOR and set flags as needed.
              if (Chip8_DisplayMemory[address] == 1)
              {
                Chip8_VRegister[0xF] = 1;
              }
              Chip8_DisplayMemory[address] ^= 1;
            }
          }
        }
      }
    }
    Chip8_DrawFlag = 1;
    Chip8_ProgramCounter += 2;
    break;

  case 0xE000:
    switch (Chip8_OpCode & 0x00FF)
    {

    // EX9E - SKP Vx
    case 0x009E:
      // Skip next instruction if key with the value of Vx is pressed.
      if (Chip8_KeyStates[Chip8_VRegister[x]] == CHIP8_KEYDOWN)
      {
        Chip8_ProgramCounter += 2;
      }
      Chip8_ProgramCounter += 2;
      break;

    // EXA1 - SKNP Vx
    case 0x00A1:
      // Skip next instruction if key with the value of Vx is not pressed.
      if (Chip8_KeyStates[Chip8_VRegister[x]] == CHIP8_KEYUP)
      {
        Chip8_ProgramCounter += 2;
      }
      Chip8_ProgramCounter += 2;
      break;

    default:
      break;
    }
    break;

  case 0xF000:
    switch (Chip8_OpCode & 0x00FF)
    {
    // FX07 - LD Vx, Chip8_DelayTimer
    case 0x0007:
      // Set Vx = Chip8_DelayTimer value.
      Chip8_VRegister[x] = Chip8_DelayTimer;
      Chip8_ProgramCounter += 2;
      break;

    // FX0A - LD Vx, K
    case 0x000A:
      // Wait for a key press, store the value of the key in Vx. All execution stops until a key is pressed,
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

    // FX15 - LD Chip8_DelayTimer, Vx
    case 0x0015:
      // Set Chip8_DelayTimer = Vx.
      Chip8_DelayTimer = Chip8_VRegister[x];
      Chip8_ProgramCounter += 2;
      break;

    // FX18 - LD Chip8_SoundTimer, Vx
    case 0x0018:
      // Set sound timer = Vx.
      Chip8_SoundTimer = Chip8_VRegister[x];
      Chip8_ProgramCounter += 2;
      break;

    // FX1E - ADD I, Vx
    case 0x001E:
      // VF is set to 1 when range overflow occurs (Chip8_IndexRegister + VX > 0xFFF), and 0 when it isn't.
      Chip8_VRegister[0xF] = 0;
      if (Chip8_IndexRegister + Chip8_VRegister[x] >= 0xFFF)
      {
        Chip8_VRegister[0xF] = 1;
      }
      Chip8_IndexRegister += Chip8_VRegister[x];
      Chip8_ProgramCounter += 2;
      break;

    // FX29 - LD F, Vx
    case 0x0029:
      // Set Chip8_IndexRegister = location of sprite for digit Vx.
      Chip8_IndexRegister = Chip8_VRegister[x] * 0x5;
      Chip8_ProgramCounter += 2;
      break;

    // FX30 - SET I,V[X]
    case 0x0030:
      // Point I to 10-byte font sprite for digit VX (only digits 0-9)
      Chip8_IndexRegister = 80 + (Chip8_VRegister[x] * 10);
      Chip8_ProgramCounter += 2;
      break;

    // FX33 - LD B, Vx
    case 0x0033:
      // store BCD representation of Vx in memory locations Chip8_IndexRegister, Chip8_IndexRegister+1, and Chip8_IndexRegister+2.
      // The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in Chip8_IndexRegister,
      // the tens digit at location I + 1, and the ones digit at location Chip8_IndexRegister + 2.
      Chip8_ProgramMemory[Chip8_IndexRegister] = Chip8_VRegister[x] / 100;
      Chip8_ProgramMemory[Chip8_IndexRegister + 1] = (Chip8_VRegister[x] / 10) % 10;
      Chip8_ProgramMemory[Chip8_IndexRegister + 2] = (Chip8_VRegister[x] % 100) % 10;
      Chip8_ProgramCounter += 2;
      break;

    // FX55 - LD [Chip8_IndexRegister], Vx
    case 0x0055:
      // The interpreter copies the values of registers V0 through Vx into memory, starting at the address in Chip8_IndexRegister.
      for (int i = 0; i <= x; i++)
      {
        Chip8_ProgramMemory[Chip8_IndexRegister + i] = Chip8_VRegister[i];
      }
      Chip8_ProgramCounter += 2;
      break;

    // FX65 - LD Vx, [I]
    case 0x0065:
      // Read registers V0 through Vx from memory starting at location I.
      for (int i = 0; i <= x; i++)
      {
        Chip8_VRegister[i] = Chip8_ProgramMemory[Chip8_IndexRegister + i];
      }
      Chip8_ProgramCounter += 2;
      break;

    case 0x0075:
      // Store the CHIP8 Registers V[0]-V[x] in the HP48 registers.
      for (int c = 0; c <= x; c++)
      {
        Chip8_HP48Registers[c] = Chip8_VRegister[c];
      }
      Chip8_ProgramCounter += 2;
      break;

    case 0x0085:
      // Read from HP48 Registers a fill the CHIP8 Registers V[0]-V[x].
      for (int c = 0; c <= x; c++)
      {
        Chip8_VRegister[c] = Chip8_HP48Registers[c];
      }
      Chip8_ProgramCounter += 2;
      break;

    default:
      break;
    }
    break;
  }

  // Is it time to decrement the Chip8_DelayTimer. (0.016666 = one second / 60 = 60hz)
  if (CurrentTime - Last_DelayUpdate > 0.01666)
  {
    if (Chip8_DelayTimer > 0)
    {
      Chip8_DelayTimer--;
    }
    Last_DelayUpdate = CurrentTime;
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
 * Tigr *screen
 *
 * Returns:
 * void.
 */
void Chip8_GetKeyStates(Tigr *screen)
{
  // 0x1, 1
  if (tigrKeyDown(screen, '1') || tigrKeyHeld(screen, '1'))
  {
    Chip8_KeyStates[0x1] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0x1] = CHIP8_KEYUP;
  }

  // 0x2, 2
  if (tigrKeyDown(screen, '2') || tigrKeyHeld(screen, '2'))
  {
    Chip8_KeyStates[0x2] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0x2] = CHIP8_KEYUP;
  }

  // 0x3, 3
  if (tigrKeyDown(screen, '3') || tigrKeyHeld(screen, '3'))
  {
    Chip8_KeyStates[0x3] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0x3] = CHIP8_KEYUP;
  }

  // 0xc, 4
  if (tigrKeyDown(screen, '4') || tigrKeyHeld(screen, '4'))
  {
    Chip8_KeyStates[0xC] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0xC] = CHIP8_KEYUP;
  }

  // 0x4,  Q
  if (tigrKeyDown(screen, 'Q') || tigrKeyHeld(screen, 'Q'))
  {
    Chip8_KeyStates[0x4] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0x4] = CHIP8_KEYUP;
  }

  // 0x5,  W
  if (tigrKeyDown(screen, 'W') || tigrKeyHeld(screen, 'W'))
  {
    Chip8_KeyStates[0x5] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0x5] = CHIP8_KEYUP;
  }

  // 0x6, E
  if (tigrKeyDown(screen, 'E') || tigrKeyHeld(screen, 'E'))
  {
    Chip8_KeyStates[0x6] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0x6] = CHIP8_KEYUP;
  }

  // 0xD, R
  if (tigrKeyDown(screen, 'R') || tigrKeyHeld(screen, 'R'))
  {
    Chip8_KeyStates[0xD] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0xD] = CHIP8_KEYUP;
  }

  // 0x7, A
  if (tigrKeyDown(screen, 'A') || tigrKeyHeld(screen, 'A'))
  {
    Chip8_KeyStates[0x7] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0x7] = CHIP8_KEYUP;
  }

  // 0x8, S
  if (tigrKeyDown(screen, 'S') || tigrKeyHeld(screen, 'S'))
  {
    Chip8_KeyStates[0x8] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0x8] = CHIP8_KEYUP;
  }

  // 0x9, D
  if (tigrKeyDown(screen, 'D') || tigrKeyHeld(screen, 'D'))
  {
    Chip8_KeyStates[0x9] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0x9] = CHIP8_KEYUP;
  }

  // 0xE, F
  if (tigrKeyDown(screen, 'F') || tigrKeyHeld(screen, 'F'))
  {
    Chip8_KeyStates[0xE] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0xE] = CHIP8_KEYUP;
  }

  // 0xA, Z
  if (tigrKeyDown(screen, 'Z') || tigrKeyHeld(screen, 'Z'))
  {
    Chip8_KeyStates[0xA] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0xA] = CHIP8_KEYUP;
  }

  // 0x0, X
  if (tigrKeyDown(screen, 'X') || tigrKeyHeld(screen, 'X'))
  {
    Chip8_KeyStates[0x0] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0x0] = CHIP8_KEYUP;
  }

  // 0xB, C
  if (tigrKeyDown(screen, 'C') || tigrKeyHeld(screen, 'C'))
  {
    Chip8_KeyStates[0xB] = CHIP8_KEYDOWN;
  }
  else
  {
    Chip8_KeyStates[0xB] = CHIP8_KEYUP;
  }

  // 0xF  V
  if (tigrKeyDown(screen, 'V') || tigrKeyHeld(screen, 'V'))
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
  Tigr *screen = NULL;
  char ROM_FileName[1024] = {'\0'};

  // Initialise the applications window
  screen = tigrWindow(CLIENTWIDTH, CLIENTHEIGHT, "Super Chip", TIGR_FIXED);

  // Clear the client window contents before we start.
  tigrClear(screen, BACKGROUND);

  // Initialise the Chip8 registers
  Chip8_Initialise();

  // No command line passed, so browse for a CHIP8 ROM file instead.
  if (argc == 1)
  {
    OpenFileDialog(ROM_FileName, sizeof(ROM_FileName));
    if (strlen(ROM_FileName) > 0)
    {
      // Load the selected ROM file.
      Chip8_LoadROM(ROM_FileName);
    }
  }

  // Load the ROM file passed on the command line
  else if (argc == 2)
  {
    Chip8_LoadROM(argv[1]);
    strcpy(ROM_FileName, argv[1]);
  }

#ifdef NDEBUG
  Console_Show("Debug Window");
#endif

  // Loop until the user exits.
  while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE))
  {
    // Clear the background
    tigrClear(screen, BACKGROUND);

    // Only emulate one cycle per feame, if NDEUG set.
#ifndef NDEBUG
    for (int n = 0; n < CHIP8TICKSPERFRAME; n++)
#endif
    {
      CurrentTime += tigrTime();

#ifdef NDEBUG
      // Disassemble the current command.
      Chip8_Disassemble();
#endif

#ifdef NDEBUG
      if (tigrKeyDown(screen, TK_RIGHT) || tigrKeyHeld(screen, TK_RIGHT))
      {
        Chip8_EmulateCPU();
      }

      if (tigrKeyDown(screen, TK_LEFT))
      {
        Chip8_ProgramCounter -= 2;
        if (Chip8_ProgramCounter <= 512)
        {
          Chip8_ProgramCounter = 512;
        }
        Chip8_Disassemble();
        Chip8_EmulateCPU();
      }

#else
      // Emulate a cpu cycle.
      Chip8_EmulateCPU();
#endif

      // Process the keypress states.
      Chip8_GetKeyStates(screen);

#ifdef NDEBUG
      // Show the chip register states.
      Chip8_ShowProgramState();
#endif

      // Reload the current rom if 'L' pressed
      if (tigrKeyDown(screen, 'L'))
      {
        Chip8_Initialise();
        Chip8_LoadROM(ROM_FileName);
      }

      // Open a different ROM file if 'O' pressed
      if (tigrKeyDown(screen, 'O'))
      {
        OpenFileDialog(ROM_FileName, sizeof(ROM_FileName));
        if (strlen(ROM_FileName) > 0)
        {
          // Load the selected ROM file.
          Chip8_Initialise();
          Chip8_LoadROM(ROM_FileName);
        }
      }
    }

    // Update the chip8 screen.
    Chip8_DrawScreen(screen);

    // Tell Tigr to update.
    tigrUpdate(screen);
  }

  // Close the window and shut down Tigr.
  tigrFree(screen);

  // Return to the OS.
  return EXIT_SUCCESS;
}
