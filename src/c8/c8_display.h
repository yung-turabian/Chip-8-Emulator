/* Copyright 2025 Punchline Software

*/

#include "c8_shared.h"

#ifndef PUNCHLINE_C8_DISPLAY_H
#define PUNCHLINE_C8_DISPLAY_H

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define SCREEN_SCALE 16

extern u8 C8_gfx[SCREEN_WIDTH * SCREEN_HEIGHT];

static inline void C8_ClearDisplay( void ) {
    memset( C8_gfx, 0, SCREEN_SIZE );
}

#endif /* PUNCHLINE_C8_DISPLAY_H */
