#include "c8_functions.h"
#include "c8_system.h"
#include "c8_display.h"

// TODO: replace log messages with NNN to actual value.
char cmd[512] = {'\0'};

/* Interface functions */
void C8_Fetch( void ) {
		opcode = ( (memory[PC] << 8) | memory[PC + 1] );
}

void C8_Execute( void ) {
		C8_Fetch();
		C8Table[(opcode & 0xF000) >> 12]();
}

/* Sub tables */
void C8_CPU_SYSTEM( void ) {
		C8System[(opcode & 0x00FF)]();		
}

void C8_CPU_MATH( void ) {
		C8Math[(opcode & 0x000F)]();
}

void C8_CPU_KEY( void ) {
		C8Key[(opcode & 0x00FF)]();
}

void C8_CPU_MEM( void ) {
		C8Mem[(opcode & 0x00FF)]();
}

/* Functions */

void cpuNULL( void ) {
		fprintf ( stderr, "[c8] Unknown opcode [0x0000]: 0x%X\n", opcode );
}

void C8_Call_CallMachine( void ) {
		fprintf( stderr, "[c8] Calling machine code is not implemented!\n" );
}

// 0x00E0: Clear screen
void C8_Display_Clear( void ) {
		strcpy( cmd, "Clearing screen" );
		C8_ClearDisplay();
		PC += 2;
}

// 0x00EE: Return from subroutine
void C8_Flow_Return( void ) {
	strcpy( cmd, "Return from subroutine." );	
	if ( sp == 0 ) {
			fprintf( stderr, "[c8] Stack underflow!\n" );
			halt = true;
	} else {
			PC = stack[--sp];
			PC += 2;
	}
}

// 0x1NNN: Jumps to address NNN.
void C8_Flow_Jump( void ) {
		//strcpy( cmd, "Jump to NNN." );
		/*if ( (opcode & 0x0FFF) == PC ) {
				fprintf(stderr, "[c8] Infinite loop detected at 0x%03X\n",
								(opcode & 0x0FFF) );
				halt = true;
				return;
		}*/
		PC = opcode & 0x0FFF;
}

// 0x2NNN: Call subroutine at NNN.
void C8_Flow_Call( void ) {
		//strcpy( cmd, "Call NNN." );	
		if ( sp >= STACK_SIZE ) {
				fprintf( stderr, "Stack Overflow!\n" );
				halt = true;
				return;
		}
		stack[sp++] = PC;
		PC = opcode & 0x0FFF;
}

// 0x3XNN: Skips the next instruction if VX == NN.
void C8_Cond_IfVXEqualsNN( void ) {
		//strcpy( cmd, "Skipping if VX == NN" );
		PC += ( V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF) ) ? 4 : 2;
}

// 0x4XNN: Skips the next instruction if VX != NN.
void C8_Cond_IfVXNotEqualsNN( void ) {
		//strcpy( cmd, "Skipping if VX != NN" );
		PC += ( V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF) ) ? 4 : 2;
}

// 0x5XY0: Skips the next instruction if VX == VY.
void C8_Cond_IfVXEqualsVY( void ) {
		//strcpy( cmd, "Skipping if VX == VY" );
		PC += ( V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4] ) ? 4 : 2;
}

// 0x6XNN: Sets VX to NN.
void C8_Const_SetVXToNN( void ) {
		//strcpy( cmd, "Set VX to NN" );	
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		PC += 2;
}

// 0x7XNN: Adds NN to VX.
void C8_Const_AddNNToVX( void ) {
		//strcpy( cmd, "Adds NN to VX." );	
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		PC += 2;
}

void C8_Assig_SetVXToVY( void ) {
		//strcpy( cmd, "Set VX to VY (value)" );	
		V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
		PC += 2;
}

void C8_Math_SetVXToVXOrVY( void ) {
		//strcpy( cmd, "Set VX to VX or VY" );	
		V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
		PC += 2;
}

void C8_Math_SetVXToVXAndVY( void ) {
		//strcpy( cmd, "Set VX to VX and VY" );	
		V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
		PC += 2;
}

void C8_Math_SetVXToVXXorVY( void ) {
		//strcpy( cmd, "Set VX to VX xor VY" );	
		V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
		PC += 2;
}

// 0x8XY4: Adds VY to VX. VF is set to 1 if there's an overflow.
void C8_Math_AddVYToVX( void ) {
		//strcpy( cmd, "Add VY to VX" );	
		//   VY                       MAX 8BIT   VX
		if ( V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]) ) {
				V[0xF] = 1; // carry
		} else {
				V[0xF] = 0;
		}
		V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
		PC += 2;
}


// 0x8XY5: Subtracts VY from VX. VF is set to 0 if there's an underflow.
void C8_Math_SubVYFromVX( void ) {
		//strcpy( cmd, "Subtract VY from VX" );	
		if ( V[(opcode & 0x0F00) >> 8] >= V[(opcode & 0x00F0) >> 4] ) {
				V[0xF] = 1; // underflow
		} else {
				V[0xF] = 0;
		}
		V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
		PC += 2;
}


// 0x8XY6: Shifts VX to the right by 1, then stores the least significant bit of VX prior to shift into VF.
void C8_Math_ShiftVXToRight( void ) {
		//strcpy( cmd, "Shift VX to the right" );	
		V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
		V[(opcode & 0x0F00) >> 8] >>= 1;
		PC += 2;
}

// 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not.
void C8_Math_SetVXToVYMinusVX( void ) {
		//strcpy( cmd, "Set VX to VY - VX" );	
		if ( V[(opcode & 0x00F0) >> 4] >= V[(opcode & 0x0F00) >> 8] ) {
				V[0xF] = 1; // underflow
		} else {
				V[0xF] = 0;
		}		
		V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
		PC += 2;
}


// 0x8XYE: Shifts VX to left by 1, then sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset. 
void C8_Math_ShiftVXToLeft( void ) {
		//strcpy( cmd, "Shift VX to left" );	
		V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x80) >> 7; // MSB
		V[(opcode & 0x0F00) >> 8] <<= 1;
		PC += 2;
}

void C8_Cond_SkipIfVXNotEqualsVY( void ) {
		//strcpy( cmd, "Skipping if VX != VY" );
		PC += (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) ? 4 : 2;
}

void C8_Mem_SetIToNNN( void ) {
		//strcpy( cmd, "Set I to NNN" );	
		I = opcode & 0x0FFF; // nibble
		PC += 2;
}

void C8_Flow_JumpToNNNPlusV0( void ) {
		//strcpy( cmd, "Jump to NNN + V0" );	
		PC = V[0] + ( opcode & 0x0FFF );
}

void C8_Rand_SetVXToRandom( void ) {
		//strcpy( cmd, "Setting VX to result of operation on random number" );
		V[(opcode & 0x0F00) >> 8] = RandomU8( m_state ) & ( opcode & 0x00FF );
		PC += 2;
}

void C8_Display_Draw( void ) {
		//strcpy( cmd, "Drawing pixel." );	
		u16 x = V[(opcode & 0x0F00) >> 8];
		u16 y = V[(opcode & 0x00F0) >> 4];
		u16 height = opcode & 0x000F;
		u16 pixel;

		V[0xF] = 0;
		for ( u16 yline = 0; yline < height; yline++ ) {
				pixel = memory[I + yline];
				for ( u16 xline = 0; xline < 8; xline++ ) { // Width of sprite can only be 8
						if ( (pixel & (0x80 >> xline)) != 0 ) {
								if ( C8_gfx[(x + xline + ((y + yline) * 64))] == 1 ) {
										V[0xF] = 1;
								}
								C8_gfx[x + xline + ((y + yline) * 64)] ^= 1;
						}

				}
		}
		PC += 2;
}

/* 							Keypad                   Keyboard
								+-+-+-+-+                +-+-+-+-+
								|1|2|3|C|                |1|2|3|4|
								+-+-+-+-+                +-+-+-+-+
								|4|5|6|D|                |Q|W|E|R|
								+-+-+-+-+       =>       +-+-+-+-+
								|7|8|9|E|                |A|S|D|F|
								+-+-+-+-+                +-+-+-+-+
								|A|0|B|F|                |Z|X|C|V|
								+-+-+-+-+                +-+-+-+-+                  */
// EX9E: Skips the next instruction if the key stored in VX is pressed
void C8_KeyOp_SkipIfPressed( void ) {
		//strcpy( cmd, "Skipping next instruciton if key in VX is pressed" );	
		PC += ( key[V[(opcode & 0x0F00) >> 8]] != 0 ) ? 4 : 2; 
}

// TODO
void C8_KeyOp_SkipIfNotPressed( void ) {
		//strcpy( cmd, "Key press is awaited." );
		halt = true;
}

// TODO
void C8_Timer_SetDelayTimer( void ) {
		//strcpy( cmd, "Set delay timer to VX." );
		delay_timer = V[(opcode & 0x0F00) >>8];
		PC += 2;

}

void C8_Sound_SetSoundTimer( void ) {
		//strcpy( cmd, "Set sound timer to VX." );
		sound_timer = V[(opcode & 0x0F00) >>8];
		PC += 2;
}

// FX1E: Adds VX to I. VF is not affected.
void C8_Mem_AddVXToI( void ) {
		//strcpy( cmd, "Adding VX to I." );
		I += V[(opcode & 0x0F00) >> 8];
		PC += 2;
}

void C8_Mem_SetLocOfSpriteVX( void ) {
		//strcpy( cmd, "Set I to location of sprite for character in VX." );	
		I = V[(opcode & 0x0F00) >> 8] * 5;
		PC += 2;
}

void C8_BCD_StoreBCDOfVXInI( void ) {
		//strcpy( cmd, "Storing binary-coded decimal of VX." );	
		memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
		memory[I + 1] = ( V[(opcode & 0x0F00) >> 8] / 10 ) % 10;
		memory[I + 2] = ( V[(opcode & 0x0F00) >> 8] % 100 ) % 10;
		PC += 2;
}

void C8_Mem_StoreV0ToVX( void ) {
		//strcpy( cmd, "Storing from V0 to VX." );
		for ( u16 i = 0; i <= ((opcode & 0x0F00) >> 8); ++i ) {
			memory[I + i] = V[i];
		}
		PC += 2;
}

void C8_Mem_FillFromV0ToVX( void ) {
		//strcpy( cmd, "Filling from V0 to VX." );
		for ( u16 i = 0; i <= ((opcode & 0x0F00) >> 8); ++i ) {
				V[i] = memory[I + i];
		}
		PC += 2;
}

/* Dispatch maps */

void (*C8Table[16])() = {
		C8_CPU_SYSTEM,

		C8_Flow_Jump, C8_Flow_Call, 
		C8_Cond_IfVXEqualsNN, C8_Cond_IfVXNotEqualsNN, C8_Cond_IfVXEqualsVY, 
		C8_Const_SetVXToNN, C8_Const_AddNNToVX,

		C8_CPU_MATH, 

		C8_Cond_SkipIfVXNotEqualsVY, C8_Mem_SetIToNNN, C8_Flow_JumpToNNNPlusV0,
		C8_Rand_SetVXToRandom, C8_Display_Draw,

		C8_CPU_KEY, C8_CPU_MEM
};

void (*C8System[0x100])() = { [0 ... 0xFF] = cpuNULL };
void (*C8Key[0x100])() = { [0 ... 0xFF] = cpuNULL };
void (*C8Math[0x10])() = { [0 ... 0xF] = cpuNULL };
void (*C8Mem[0x100])() = { [0 ... 0xFF] = cpuNULL };

void C8_InitChip8Compliance( void ) {

		C8System[0] = C8_Call_CallMachine;
		C8System[0xE0] = C8_Display_Clear;
		C8System[0xEE] = C8_Flow_Return;

		C8Math[0x0] = C8_Assig_SetVXToVY;
		C8Math[0x1] = C8_Math_SetVXToVXOrVY;
		C8Math[0x2] = C8_Math_SetVXToVXAndVY;
		C8Math[0x3] = C8_Math_SetVXToVXXorVY;
		C8Math[0x4] = C8_Math_AddVYToVX;
		C8Math[0x5] = C8_Math_SubVYFromVX;
		C8Math[0x6] = C8_Math_ShiftVXToRight;
		C8Math[0x7] = C8_Math_SetVXToVYMinusVX;
		C8Math[0xE] = C8_Math_ShiftVXToLeft;

		C8Key[0x9E] = C8_KeyOp_SkipIfPressed;
		C8Key[0xA1] = C8_KeyOp_SkipIfNotPressed;

		C8Mem[0x15] = C8_Timer_SetDelayTimer;
		C8Mem[0x18] = C8_Sound_SetSoundTimer;
		C8Mem[0x1E] = C8_Mem_AddVXToI;
		C8Mem[0x29] = C8_Mem_SetLocOfSpriteVX;
		C8Mem[0x33] = C8_BCD_StoreBCDOfVXInI;
		C8Mem[0x55] = C8_Mem_StoreV0ToVX;
		C8Mem[0x65] = C8_Mem_FillFromV0ToVX;
}
