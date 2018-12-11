#ifndef CHIP8_HEADER
	#define CHIP8_HEADER
	
	// Current Opcode
	unsigned short	opcode;			// Current opcode


	// Chip8 Memory map
	// 0x000-0x1FF - Chip 8 interpreter
	// 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
	// 0x200-0xFFF - Program ROM and work RAM

	unsigned char	MEMORY[4096];			// Chip 8's Memory
		
	// Display memory
	unsigned char 	DISPLAY[2048];			// Chip 8 Display (64 * 32)
//	unsigned char 	HIRES_DISPLAY[4096];	// Chip 8 Display (64 * 64)

	// Registers
	unsigned char	V[16];					// Chip8's 16 Registers
	unsigned short	INDEX;					// Index register
	unsigned short	PC;						// Program counter
                                	
	// Timers                   	
	unsigned char	DELAY;					// Delay Timer
	unsigned char	BUZZER;					// Sound Timer
                                	
	// Stack                    	
	unsigned short	STACK[16];				// Chip8's stack
	unsigned short	SP;						// stack pointer
                                	
	// Keys                     	
	unsigned char 	KEY[16];				// Store key states

	// Screen update flag
	unsigned char 	drawflag;				// Okay to update screen 
	
	
	long framecounter;
	
	
	//unsigned char	HiresMode;	// Okay to use Chip8 Hi-Res display

	// Chip 8 default fonts
	unsigned char chip8_fontset[80] =  {
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
#endif
