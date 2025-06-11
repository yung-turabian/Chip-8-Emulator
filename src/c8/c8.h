/* Copyright 2025 Punchline Software

   Entrypoint.
*/

#ifndef PUNCHLINE_CHIP8_H
#define PUNCHLINE_CHIP8_H

#include "c8_display.h"

u8 C8_Setup( void );
u8 C8_LoadGame( const char *path );
void C8_EmulateCycle( void );

extern u8 C8_key[16];

#endif /* PUNCHLINE_CHIP8_H */
