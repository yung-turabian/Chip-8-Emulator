/* Copyright 2025 Punchline Software

*/

#ifndef PUNCHLINE_CHIP8_H
#define PUNCHLINE_CHIP8_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "framework.h"
#include "log.h"

typedef uint8_t u8;
typedef uint16_t u16;

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define SCREEN_SCALE 16

#define MEMORY_LOCATIONS 4096
#define INTERPRETER_SIZE 512
#define NUMBER_OF_REGISTERS 16
#define STACK_SIZE 16

u16 opcode;
u8 memory[MEMORY_LOCATIONS];
u8 V[NUMBER_OF_REGISTERS]; // CPU registers

u16 I; // Index register
u16 pc; // Program counter
/* 
 * 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
 * 0x050-0x0A0 - Used for the built in 4x6 pixel font set (0-F)
 * 0x200-0xFFF - Program ROM and work ROM
 */

u8 gfx[SCREEN_WIDTH * SCREEN_HEIGHT] = { 0 };

u8 delay_timer;
u8 sound_timer; // Buzzer sounds whenever the sound timer reaches 0

u16 stack[STACK_SIZE];
u16 sp;

u8 key[16] = {0};

bool draw_flag = false;

u8 chip8_fontset[80] = {
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

u8 C8Setup( void );
u8 C8LoadGame( const char *path );
void C8EmulateCycle( void );

u8 C8Setup( void ) {
		Log( LTRACE, "Booting c8..." );
		pc = 0x200;
		opcode = 0, I = 0, sp = 0;

		
		for ( u16 i = 0; i < MEMORY_LOCATIONS; i++ ) {
				memory[i] = 0;
		}

		for ( u16 i = 0; i < NUMBER_OF_REGISTERS; i++ ) {
				V[i] = 0;
		}

		for ( u16 i = 0; i < STACK_SIZE; i++ ) {
				stack[i] = 0;
		}

		// Clear display

		for ( u8 i = 0; i < 80; ++i ) {
				memory[i] = chip8_fontset[i];
		}

		// Reset timers
		
		return 0;
}

// TODO: Use libreis fileystem code.
u8 C8LoadGame( const char* path ) {
		if ( path == NULL ) {
				fprintf( stderr, "[c8] Invalid rom path name.\n" );
				return 0;
		}

		char buf[MEMORY_LOCATIONS - INTERPRETER_SIZE];
		u16 buf_siz = MEMORY_LOCATIONS - INTERPRETER_SIZE;

		FILE *rom = fopen( path, "rb" );
		if ( !rom ) {
				perror( "[c8] Error reading ROM" );
				return EXIT_FAILURE;
		}

		size_t ret = fread( buf, sizeof(*buf), buf_siz, rom );
		if ( ferror(rom) ) {
				fprintf( stderr, "[c8] fread() failed: %zu\n", ret );
				exit( EXIT_FAILURE );
		} else if ( feof(rom) ) {
#ifdef DEBUG 
				Log( LTRACE, "Succesfully loaded rom: %s! \n\tSize: %zu", path, ret );
#endif
		}

		for ( u16 i = 0; i < buf_siz; ++i ) {
				memory[i + 512] = buf[i]; // 0x200 == 512
		}

		fclose( rom );
		return 0;
}

void C8EmulateCycle( void ) {
		// Fetch Opcode
		u16 opcode = memory[pc] << 8 | memory[pc + 1];

		// Decode Opcode
		// compare to opcode table to get meaning
		switch ( opcode & 0xF000 ) {
				// Some opcodes //

				case 0x0000:
						switch ( opcode & 0x000F ) {
								case 0x0000: // 0x00E0: Clear screen
										// Excute
								break;

								case 0x000E: // 0x00EE: Return from subroutine 
										// Excute
								break;

								default:
										fprintf ( stderr, "[c8] Unknown opcode [0x0000]: 0x%X\n", opcode );
						}
				break;

				case 0x2000:
						stack[sp++] = pc;
						pc = opcode & 0x0FFF;
				break;

				case 0xA000: // ANNN: Sets I to the address NNN
						I = opcode & 0x0FFF; // nibble
						pc += 2;
				break;

				case 0x0004:
						if ( V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
								V[0xF] = 1; // carry
						} else {
								V[0xF] = 0;
						}
						V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
						pc += 2;
				break;

				case 0x0033:
						memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
						memory[I + 1] = ( V[(opcode & 0x0F00) >> 8] / 10 ) % 10;
						memory[I + 2] = ( V[(opcode & 0x0F00) >> 8] % 100 ) % 10;
						pc += 2;
				break;

				case 0xD000:
						u16 x = V[(opcode & 0x0F00) >> 8];
						u16 y = V[(opcode & 0x00F0) >> 4];
						u16 height = opcode & 0x000F;
						u16 pixel;

						V[0xF] = 0;
						for ( u16 yline = 0; yline < height; yline++ ) {
								pixel = memory[I + yline];
								for ( u16 xline = 0; xline < 8; xline++ ) { // Width of sprite can only be 8
										if ( (pixel & (0x80 >> xline)) != 0 ) {
												if ( gfx[(x + xline + ((y + yline) * 64))] == 1 ) {
														V[0xF] = 1;
												}
												gfx[x + xline + ((y + yline) * 64)] ^= 1;
										}

								}
						}

						draw_flag = true;
						pc += 2;
				break;

				case 0xE000:
				/* 
								Keypad                   Keyboard
								+-+-+-+-+                +-+-+-+-+
								|1|2|3|C|                |1|2|3|4|
								+-+-+-+-+                +-+-+-+-+
								|4|5|6|D|                |Q|W|E|R|
								+-+-+-+-+       =>       +-+-+-+-+
								|7|8|9|E|                |A|S|D|F|
								+-+-+-+-+                +-+-+-+-+
								|A|0|B|F|                |Z|X|C|V|
								+-+-+-+-+                +-+-+-+-+
				 */
						switch ( opcode & 0x00FF ) {
								// EX9E: Skips the next instruction
								// if the key stored in VX is pressed
								case 0x009E:
										if ( key[V[(opcode & 0x0F00) >> 8]] != 0 ) {
												pc += 4;
										} else {
												pc += 2;
										}
								break;
						}
				break;
				// TODO: Add the rest //

				default:
						fprintf( stderr, "[c8] Unknow opcode: 0x%X\n", opcode );
		}

		// pc += 2 unless jumping to a certain address in memory or if calling a subroutine, in which case store the pc in stack; if next opcode should be skipped increment by four
		
		// Update timers
		if ( delay_timer > 0 ) {
				--delay_timer;
		}

		if ( sound_timer > 0 ) {
				if ( sound_timer == 1) {
						printf( "BEEP!\n" );
						--sound_timer;
				}
		}
}

#endif /* PUNCHLINE_CHIP8_H */
