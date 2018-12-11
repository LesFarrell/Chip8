#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "chip8.h"
#include "raylib.h"

char buffer[1024] = {'\0'};

void chip8_initialise(void)
{
    PC          = 0x200;    // Program counter
    INDEX       = 0;        // Index register
    DELAY       = 0;        // Delay timer
    BUZZER      = 0;        // Sound timer
    SP          = 0;        // Stack pointer
    drawflag    = 1;        // Screen update flag
    opcode      = 0;        // Current op code


    // Clear the display memory
    for (int i = 0; i < 2048; ++i) DISPLAY[i] = 0;

    // Clear stack memory
    for (int i = 0; i < 16; ++i) STACK[i] = 0;

    // Clear the registers
    for (int i = 0; i < 16; ++i) KEY[i] = V[i] = 0;

    // Clear main memory
    for (int i = 0; i < 4096; ++i) MEMORY[i] = 0;

    // Load the default chip8 fonts
    for (int i = 0; i < 80; ++i) MEMORY[i] = chip8_fontset[i];

    // Initialise the random seed
    srand(time(NULL));

    // Reset the draw flag
    drawflag = 0;
}


void chip8_drawgraphics(void)
{
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
            if (DISPLAY[ (y * 64) + x] != 0) {
                DrawRectangle (x * 10, y * 10, 10, 10, WHITE);
            } else {
                DrawRectangle (x * 10, y * 10, 10, 10, BLUE);
            }
        }
	}
}


int chip8_load(char *filename)
{
	FILE *fp;
	unsigned short pos = 512;
	unsigned char ch;
	
	fp = fopen (filename, "rb");
    if (fp == NULL) return 0;

	while (!feof(fp))
	{
		ch = fgetc(fp);		
		MEMORY[pos] = ch;
		pos++;
		if (pos >= 4096) break;
	}
	fclose(fp);
	return 0;
}


void chip8_showregisters(void)
{
    int i = 0;
    int y = 40;
    char string[100];

    sprintf(string, "PC	: %d", PC);
    DrawText (string, 650, 10, 10, WHITE);
    
    sprintf(string, "SP	: %d", SP);
    DrawText (string, 650, 20, 10, WHITE);
    
    sprintf(string, "INDEX	: %d", INDEX);
    DrawText (string, 650, 30, 10, WHITE);

    sprintf(string, "DELAY	: %d", DELAY);
    DrawText (string, 730, 10, 10, WHITE);

    sprintf(string, "BUZZER	: %d", BUZZER);
    DrawText (string, 730, 20, 10, WHITE);


    for (i = 0; i < 16; i++) {
        sprintf(string, "V%02d : %02d", i, V[i]);
        DrawText (string, 650, y, 10, WHITE);
        y = y + 10;
    }

	y = 40;
    for (i = 0; i < 16; i++) {
        sprintf(string, "Stack[%02d] : %02d", i, STACK[i]);
        DrawText (string, 730, y, 10, WHITE);
        y = y + 10;
    }
}


void chip8_emulatecpu(void)
{
    unsigned char x, y;
    unsigned short kk, nnn, n;
    unsigned short height;
    unsigned short pixel;
	double	timer;
    char string[100] = {'\0'};
    int FramesPerSecond = GetFPS();

    // Grab the next opcode
    opcode = ((MEMORY[PC] << 8) + MEMORY[PC + 1]);

    // Process the opcode
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x000F) {
                
                // 00E0 - CLS
                // Clear the display.
                case 0x0000:
                    sprintf(buffer, "%X\t : 00E0\t - \tCLS", opcode);
                    for (int c = 0; c < 2048; c++) DISPLAY[c] = 0;
                    drawflag = 1;
                    PC += 2;
                    break;


                // 00EE - RET
                // Return from a subroutine.
                // The interpreter sets the program counter to the address at the top of the stack,
                // then subtracts 1 from the stack pointer.
                case 0x000E:
                    sprintf(buffer, "%X\t\t : 00EE\t - \tRET", opcode);
					SP--;
					PC = STACK[SP];
					PC += 2;
                    break;

                default:
                    sprintf(buffer, "Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;


        // 1nnn - JP addr
        // Jump to location nnn.
        // The interpreter sets the program counter to nnn.
        case 0x1000:
            nnn = (opcode & 0x0FFF);
            sprintf(buffer, "%X\t : 1nnn\t - \tJP %d", opcode, nnn);
            PC = nnn;
            break;


        // 2nnn - CALL addr
        // Call subroutine at nnn.
        // The interpreter increments the stack pointer, then puts the current PC on the top of the stack.
        // The PC is then set to nnn.
        case 0x2000:
            nnn = (opcode & 0x0FFF);
            sprintf(buffer,  "%X\t : 2nnn\t - \tCALL %d", opcode, nnn);
			STACK[SP] = PC;
			SP++;
            PC = nnn;
            break;


        // 3xkk - SE Vx, byte
        // Skip next instruction if Vx = kk.
        // The interpreter compares register Vx to kk, and if they are equal,
        // increments the program counter by 2.
        case 0x3000:
            x = (opcode & 0x0F00) >> 8;
            kk = (opcode & 0x00FF);
            sprintf(buffer,  "%X\t : 3xkk\t - \tSE V%d, %d", opcode, x, kk);
            if (V[x] == kk) PC += 2;
			PC += 2;
            break;


        // 4xkk - SNE Vx, byte
        // Skip next instruction if Vx != kk.
        // The interpreter compares register Vx to kk and if they are not equal,
        // increments the program counter by 2.
        case 0x4000:
            x = (opcode & 0x0F00) >> 8;
            kk = (opcode & 0x00FF);
            sprintf(buffer, "%X\t : 4xkk\t - \tSNE V%d, %d", opcode, x, kk);
            if (V[x] != kk) PC += 2;
            PC += 2;
            break;


        // 5xy0 - SE Vx, Vy
        // Skip next instruction if Vx = Vy.
        // The interpreter compares register Vx to register Vy and if they are equal,
        // increments the program counter by 2.
        case 0x5000:
            x = (opcode & 0x0F00) >> 8;
            y = (opcode & 0x00F0) >> 4;
            sprintf(buffer, "%X\t : 5xy0\t - \tSE V%d, V%d", opcode, x, y);
            if (V[x] == V[y]) PC += 2;
            PC += 2;
            break;


        // 6xkk - LD Vx, byte
        // Set Vx = kk.
        // The interpreter puts the value kk into register Vx.
        case 0x6000:
            x = (opcode & 0x0F00) >> 8;
            kk = (opcode & 0x00FF);
            sprintf(buffer, "%X\t : 6xkk\t - \tLD V%d, %d", opcode, x, kk);
            V[x] = kk;
            PC += 2;
            break;  


        // 7xkk - ADD Vx, byte
        // Set Vx = Vx + kk.
        // Adds the value kk to the value of register Vx, then stores the result in Vx.
        case 0x7000:
            x = (opcode & 0x0F00) >> 8;
            kk = (opcode & 0x00FF);
            sprintf(buffer, "%X\t : 7xkk\t - \tADD V%d, %d", opcode, x, kk);
			V[x] += kk;
            PC += 2;
            break;


        case 0x8000:

            switch (opcode & 0x000F) {
                
                // 8xy0 - LD Vx, Vy
                // Set Vx = Vy.
                // stores the value of register Vy in register Vx.
                case 0x000:
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    sprintf(buffer, "%X\t : 8xy0\t - \tLD V%d, V%d", opcode, x, y);
                    V[x] = V[y];
                    PC += 2;
                    break;


                // 8xy1 - OR Vx, Vy
                // Set Vx = Vx OR Vy.
                // Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
                // A bitwise OR compares the corrseponding bits from two values,
                // and if either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0.
                case 0x001:
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    sprintf(buffer, "%X\t : 8xy1\t - \tOR V%d, V%d", opcode, x, y);
                    V[x] |= V[y];
                    PC += 2;
                    break;


                // 8xy2 - AND Vx, Vy
                // Set Vx = Vx AND Vy.
                // Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
                // A bitwise AND compares the corrseponding bits from two values,
                // and if both bits are 1, then the same bit in the result is also 1. Otherwise, it is 0.
                case 0x002:
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    sprintf(buffer, "%X\t : 8xy2\t - \tAND V%d, V%d", opcode, x, y);
                    V[x] &= V[y];
                    PC += 2;
                    break;


                // 8xy3 - XOR Vx, Vy
                // Set Vx = Vx XOR Vy.
                // Performs a bitwise exclusive OR on the values of Vx and Vy,
                // then stores the result in Vx.
                // An exclusive OR compares the corresponding bits from two values,
                // and if the bits are not both the same then the corresponding bit
                // in the result is set to 1. Otherwise, it is 0.
                case 0x003:
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    sprintf(buffer, "%X\t : 8xy3\t - \tXOR V%d, V%d", opcode, x, y);
                    V[x] ^= V[y];
                    PC += 2;
                    break;


                // 8xy4 - ADD Vx, Vy
                // Set Vx = Vx + Vy, set VF = carry.
                // The values of Vx and Vy are added together.
                // If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0.
                // Only the lowest 8 bits of the result are kept, and stored in Vx.
                case 0x0004:
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    sprintf(buffer, "%X\t : 8xy4\t - \tADD V%d, V%d", opcode, x, y);
                    if (V[y] > (255 - V[x])) V[0xF] = 1; else V[0xF] = 0;
                    V[x] += V[y];
                    PC += 2;
                    break;


                // 8xy5 - SUB Vx, Vy
                // Set Vx = Vx - Vy, set VF = NOT borrow.
                // If Vx > Vy, then VF is set to 1, otherwise 0.
                // Then Vy is subtracted from Vx, and the results stored in Vx.
                case 0x0005:
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    sprintf(buffer, "%X\t : 8xy5\t - \tSUB V%d, V%d", opcode, x, y);
                    if (V[x] > V[y]) V[0xF] = 1; else V[0xF] = 0;
                    V[x] -= V[y];
                    PC += 2;
                    break;


                // 8xy6 - SHR Vx {, Vy}
                // Set Vx = Vx SHR 1.
                // If the least-significant bit of Vx is 1, then VF is set to 1,
                // otherwise 0. Then Vx is divided by 2.
                case 0x0006:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : 8xy6\t - \tSHR V%d {, V%d}", opcode, x);
                    V[0xF] = V[x] & 0x1;
                    V[x] = V[x] / 2;
                    PC += 2;
                    break;


                // 8Xy7 - Subn Vx, Vy
                // Set Vx = Vy - Vx, set VF = NOT borrow.
                // If Vy > Vx, then VF is set to 1, otherwise 0.
                // Then Vx is subtracted from Vy, and the results stored in Vx.
                case 0x0007:
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    sprintf(buffer, "%X\t : 8Xy7\t - \tSubn V%d, V%d", opcode, x, y);
                    if (V[y] > V[x]) V[0xF] = 1; else V[0xF] = 0;
                    V[x] = V[y] - V[x];
                    PC += 2;
                    break;


                // 8xyE - SHL Vx {, Vy}
                // Set Vx = Vx SHL 1.
                // If the most-significant bit of Vx is 1, then VF is set to 1,
                // otherwise to 0. Then Vx is multiplied by 2.
                case 0x000E:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : 8xyE\t - \tSHL V%d {, V%d}", opcode, x);
                    V[0xF] = V[x] >> 7;
                    V[x] = V[x] * 2;
                    PC += 2;
                    break;
                    
            }
			break;


        // 9xy0 - SNE Vx, Vy
        // Skip next instruction if Vx != Vy.
        // The values of Vx and Vy are compared and if they are not equal,
        // the program counter is increased by 2.
        case 0x9000:
            x = (opcode & 0x0F00) >> 8;
            y = (opcode & 0x00F0) >> 4;
            sprintf(buffer, "%X\t : 9xy0\t - \tSNE V%d, V%d", opcode, x, y);
            if (V[x] != V[y]) PC += 2;
            PC += 2;
            break;


        // Annn - LD I, addr
        // Set INDEX = nnn.
        // The value of register INDEX is set to nnn.
        case 0xA000:
            nnn = (opcode & 0x0FFF);
            sprintf(buffer, "%X\t : Annn\t - \tLD I, %d", opcode, nnn);
            INDEX = nnn;
            PC += 2;
            break;


        // Bnnn - JP V0, addr
        // Jump to location nnn + V0.
        // The program counter is set to nnn plus the value of V0.
        case 0xB000:
            nnn = (opcode & 0x0FFF);
            sprintf(buffer, "%X\t : Bnnn\t - \tJP V0, %d", opcode, nnn);
            PC = nnn + V[0];
            break;


        // Cxkk - RND Vx, byte
        // Set Vx = random byte AND kk.
        // The interpreter generates a random number from 0 to 255,
        // which is then ANDed with the value kk. The results are stored in Vx.
        // See instruction 8xy2 for more information on AND.
        case 0xC000:
            x = (opcode & 0x0F00) >> 8;
            kk = (opcode & 0x00FF);
            sprintf(buffer, "%X\t : Cxkk\t - \tRND V%d, %d", opcode, x, kk);
            V[x] = (rand() % 256) & kk;
            PC += 2;
            break;


        // Dxyn - DRW Vx, Vy, nibble
        // Display n-byte sprite starting at memory location INDEX at (Vx, Vy), set VF = collision.
        // The interpreter reads n bytes from memory, starting at the address stored in I.
        // These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
        // Sprites are XORed onto the existing screen.
        // If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
        // If the sprite is positioned so part of it is outside the coordinates of the display,
        // it wraps around to the opposite side of the screen.
        case 0xD000:
            x = (opcode & 0x0F00) >> 8;
            y = (opcode & 0x00F0) >> 4;
            height = opcode & 0x000F;
            sprintf(buffer, "%X\t : Dxyn\t - \tDRW V%d, V%d, %d", opcode, x, y, height);
            pixel = 0;
            V[0xF] = 0;
            for (int yline = 0;  yline < height;  yline++) {
                pixel = MEMORY[INDEX + yline];
                for (int xline = 0;  xline < 8;  xline++) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        if (DISPLAY[(V[x] + xline + ((V[y] + yline) * 64))] == 1) V[0xF] = 1;
                        DISPLAY[V[x] + xline + ((V[y] + yline) * 64)] ^= 1;
                    }
                }
            }
            drawflag = 1;
            PC += 2;
            break;


        case 0xE000:
            switch (opcode & 0x00FF) {
                // Ex9E - SKP Vx
                // Skip next instruction if key with the value of Vx is pressed.
                // Checks the keyboard, and if the key corresponding to the value of Vx
                // is currently in the down position,
                // PC is increased by 2.
                case 0x009E:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : Ex9E\t - \tSKP V%d", opcode, x);
                    if (KEY[V[x]] == 1) PC += 2;
                    PC += 2;
                    break;


                // ExA1 - SKNP Vx
                // Skip next instruction if key with the value of Vx is not pressed.
                // Checks the keyboard, and if the key corresponding to the value of Vx
                // is currently in the up position,
                // PC is increased by 2.
                case 0x00A1:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : ExA1\t - \tSKNP V%d", opcode, x);
                    if (KEY[V[x]] == 0) PC += 2;
                    PC += 2;
                    break;
            }
			break;
			
			
        case 0xF000:
            switch (opcode & 0x00FF) {
                // Fx07 - LD Vx, DELAY
                // Set Vx = delay timer value.
                // The value of DELAY is placed into Vx.
                case 0x0007:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : Fx07\t - \tLD V%d, DELAY", opcode, x);
                    V[x] = DELAY;
                    PC += 2;
                    break;


                // Fx0A - LD Vx, K
                // Wait for a key press, store the value of the key in Vx.
                // All execution stops until a key is pressed,
                // then the value of that key is stored in Vx.
                case 0x000A:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : Fx0A\t - \tLD V%d, K", opcode, x);
                    int keyPress = 0;
                    for (int i = 0; i < 16; i++) {
                        if (KEY[i] != 0) {
                            V[x] = i;
                            keyPress = 1;
                        }
                    }
                    // If we didn't received a keypress, skip this cycle and try again.
                    if (!keyPress) return;
                    PC += 2;
                    break;


                // Fx15 - LD DELAY, Vx
                // Set delay timer = Vx.
                // DELAY is set equal to the value of Vx.
                case 0x0015:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : Fx15\t - \tLD DELAY, V%d", opcode, x);
                    DELAY = V[x];
                    PC += 2;
                    break;


                // Fx18 - LD BUZZER, Vx
                // Set sound timer = Vx.
                // BUZZER is set equal to the value of Vx.
                case 0x0018:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : Fx18\t - \tLD BUZZER, V%d", opcode, x);
                    BUZZER = V[x];
                    PC += 2;
                    break;


                // Fx1E - ADD I, Vx
                // Set INDEX = INDEX + Vx.
                // The values of I and Vx are added, and the results are stored in I.
                case 0x001E:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : Fx1E\t - \tADD I, V%x", opcode, x);
                    // VF is set to 1 when range overflow (INDEX + VX > 0xFFF), and 0 when there isn't.
                    if (INDEX + V[x] > 0xFFF) V[0xF] = 1; else V[0xF] = 0;
                    INDEX += V[x];
                    PC += 2;
                    break;


                // Fx29 - LD F, Vx
                // Set INDEX = location of sprite for digit Vx.
                // The value of INDEX is set to the location for the hexadecimal sprite
                // corresponding to the value of Vx.
                // See section 2.4, Display, for more information on the Chip-8 hexadecimal font.
                case 0x0029:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : Fx29\t - \tLD F, V%d", opcode, x);
                    INDEX = V[x] * 0x5;
                    PC += 2;
                    break;


                // Fx33 - LD B, Vx
                // store BCD representation of Vx in memory locations INDEX, INDEX+1, and INDEX+2.
                // The interpreter takes the decimal value of Vx,
                // and places the hundreds digit in memory at location in INDEX,
                // the tens digit at location I+1, and the ones digit at location INDEX + 2.
                case 0x0033:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : Fx33\t - \tLD B, V%d", opcode, x);
                    MEMORY[INDEX]     =  V[x] / 100;
                    MEMORY[INDEX + 1] = (V[x] / 10) % 10;
                    MEMORY[INDEX + 2] = (V[x] % 100) % 10;
                    PC += 2;
                    break;


                // Fx55 - LD [INDEX], Vx
                // store registers V0 through Vx in memory starting at location INDEX.
                // The interpreter copies the values of registers V0 through Vx into memory,
                // starting at the address in INDEX.
                case 0x0055:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : Fx55\t - \tLD [I], V%d", opcode, x);
                    for (int i = 0; i <= x; i++) MEMORY[INDEX + i] = V[i];
                    INDEX = INDEX + x + 1;
                    PC += 2;
                    break;


                // Fx65 - LD Vx, [I]
                // Read registers V0 through Vx from memory starting at location I.
                // The interpreter reads values from memory starting at location I
                // into registers V0 through Vx.
                case 0x0065:
                    x = (opcode & 0x0F00) >> 8;
                    sprintf(buffer, "%X\t : Fx65\t - \tLD V%d, [I]", opcode, x);
                    for (int i = 0; i <= x; i++) V[i] = MEMORY[INDEX + i];
                    PC += 2;
                    break;
            }
			break;
			
			
        default:
			printf ("Unknown opcode %X\n", opcode);
            break;
    }
    
//	if ( framecounter - FramesPerSecond  < 60)
	{
 
    	if (DELAY > 0) DELAY = DELAY - 1 ;
	    if (BUZZER > 0) BUZZER = BUZZER - 1;
		sprintf(buffer, "%X\t Frame: %d\n", framecounter);
		framecounter=0;
	}	
//	framecounter++;

}


void chip8_getkeys(void)
{
    if (IsKeyDown(KEY_ONE) 	|| IsKeyPressed(KEY_ONE))	KEY[0x1] = 1; else KEY[0x1] = 0;
    if (IsKeyDown(KEY_TWO) 	|| IsKeyPressed(KEY_TWO)) 	KEY[0x2] = 1; else KEY[0x2] = 0;
    if (IsKeyDown(KEY_THREE)|| IsKeyPressed(KEY_THREE)) KEY[0x3] = 1; else KEY[0x3] = 0;
    if (IsKeyDown(KEY_FOUR)	|| IsKeyPressed(KEY_FOUR)) 	KEY[0xC] = 1; else KEY[0xC] = 0;
    if (IsKeyDown(KEY_Q)	|| IsKeyPressed(KEY_Q)) 	KEY[0x4] = 1; else KEY[0x4] = 0;
    if (IsKeyDown(KEY_W)	|| IsKeyPressed(KEY_W)) 	KEY[0x5] = 1; else KEY[0x5] = 0;
    if (IsKeyDown(KEY_E)	|| IsKeyPressed(KEY_E)) 	KEY[0x6] = 1; else KEY[0x6] = 0;
    if (IsKeyDown(KEY_R)	|| IsKeyPressed(KEY_R)) 	KEY[0xD] = 1; else KEY[0xD] = 0;
    if (IsKeyDown(KEY_A)	|| IsKeyPressed(KEY_A)) 	KEY[0x7] = 1; else KEY[0x7] = 0;
	if (IsKeyDown(KEY_S)	|| IsKeyPressed(KEY_S)) 	KEY[0x8] = 1; else KEY[0x8] = 0;
    if (IsKeyDown(KEY_D)	|| IsKeyPressed(KEY_D)) 	KEY[0x9] = 1; else KEY[0x9] = 0;
    if (IsKeyDown(KEY_F)	|| IsKeyPressed(KEY_F)) 	KEY[0xE] = 1; else KEY[0xE] = 0;
    if (IsKeyDown(KEY_Z)	|| IsKeyPressed(KEY_Z)) 	KEY[0xA] = 1; else KEY[0xA] = 0;
    if (IsKeyDown(KEY_X)	|| IsKeyPressed(KEY_X)) 	KEY[0x0] = 1; else KEY[0x0] = 0;
    if (IsKeyDown(KEY_C)	|| IsKeyPressed(KEY_C)) 	KEY[0xB] = 1; else KEY[0xB] = 0;
    if (IsKeyDown(KEY_V)	|| IsKeyPressed(KEY_V)) 	KEY[0xF] = 1; else KEY[0xF] = 0;
}


int main(int argc, char *argv[])
{
	int debug = 0;
    InitWindow (820, 320, "Chip8 Virtual Machine");
    char * Filename[1024] = {'\0'};

	// Initialise the Chip8 registers
    chip8_initialise();

	// Load a rom
	if (argc == 2) chip8_load(argv[1]); else chip8_load("logo.ch8");
    

    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground (BLACK);
        

		if (IsKeyDown(KEY_P)) debug = 1;
		if (IsKeyDown(KEY_O)) debug = 0;
		if (debug == 1)
		{
			if (IsKeyPressed(KEY_SPACE)) 
			{
        		// Emulate one cpu cycle
	        	chip8_emulatecpu();
			}
		}		
		else 
		{
        	// Emulate one cpu cycle
	        chip8_emulatecpu();
		}


        // store the keypress states 
	    chip8_getkeys();


      	// Emulate one cpu cycle
		chip8_emulatecpu();
              
		
        // If the draw flag is set update the screen
		if (drawflag == 1) chip8_drawgraphics();


        chip8_showregisters();
        DrawText (FormatText("[%d]:\t%s",PC,buffer), 645, 215, 10, YELLOW);
		// DrawFPS(700,225);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
