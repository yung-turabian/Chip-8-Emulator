#include "c8.h"
#include "c8_system.h"
#include "c8_functions.h"

u16 opcode;
u8 memory[MEMORY_LOCATIONS];
u8 V[NUMBER_OF_REGISTERS]; // CPU registers
u16 I; // Index register
u16 PC; // Program counter
/* 
 * 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
 * 0x050-0x0A0 - Used for the built in 4x6 pixel font set (0-F)
 * 0x200-0xFFF - Program ROM and work ROM
 */

u8 delay_timer;
u8 sound_timer; // Buzzer sounds whenever the sound timer reaches 0

u16 stack[STACK_SIZE];
u16 sp;

u8 C8_key[16] = {0};

bool halt = false;

TwistState *m_state;

u8 C8_gfx[SCREEN_WIDTH * SCREEN_HEIGHT] = { 0 };
u8 SC8_gfx[(SCREEN_WIDTH * 2) * (SCREEN_HEIGHT * 2)] = { 0 };

u8 C8_screen_scale = 16;
u32 C8_screen_size;


void C8_SetPixel( u16 x, u16 y ) {
		u16 i = y * SCREEN_WIDTH + x;
		C8_gfx[i] = 1;
}

u8 C8_GetPixel( u16 x, u16 y ) {
		u16 i = y * SCREEN_WIDTH + x;
		return C8_gfx[i];
}

void C8_DrawMenu( void ) { }

u8 C8_Setup( void ) {
		fprintf( stdout, "[c8] Booting c8...\n" );
		PC = 0x200;
		opcode = 0, I = 0, sp = 0;

		
		for ( int i = 0; i < MEMORY_LOCATIONS; i++ ) {
				memory[i] = 0;
		}

		for ( int i = 0; i < NUMBER_OF_REGISTERS; i++ ) {
				V[i] = 0;
		}

		for ( int i = 0; i < STACK_SIZE; i++ ) {
				stack[i] = 0;
		}

		C8_screen_size = highres_mode ? (SCREEN_WIDTH * SCREEN_HEIGHT) * 4 : (SCREEN_WIDTH * SCREEN_HEIGHT);	

		C8_ClearDisplay();

		C8_InitChip8Compliance();

		// Load fontset
		for ( u8 i = 0; i < 80; ++i ) {
				memory[i] = chip8_fontset[i];
		}

		delay_timer = 60;
		sound_timer = 60;
		
		m_state = (TwistState*)MALLOC( sizeof(TwistState) );
		InitMersenneState( m_state, 19650218UL );
		
		return 0;
}

// TODO: Use libreis fileystem code.
u8 C8_LoadGame( const char* path ) {
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

void C8_EmulateCycle( void ) {
		// Update timers
		if ( delay_timer > 0 ) {
				--delay_timer;
		}

		if ( sound_timer > 0 ) {
				if ( sound_timer == 1) {
						printf( "\aBEEP!\n" );
				}
				--sound_timer;
		}
		
		// Instructions per frame
		for ( int i = 0; i < 8; i++ ) {
				if ( !halt ) { 
						C8_Execute();
				}
		}

		memset( C8_key, 0, 16 );
}
