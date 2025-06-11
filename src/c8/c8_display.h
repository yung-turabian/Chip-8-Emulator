/* Copyright 2025 Punchline Software

*/

#include "c8_shared.h"

#include <stdbool.h>
#include <string.h>

#ifndef PUNCHLINE_C8_DISPLAY_H
#define PUNCHLINE_C8_DISPLAY_H

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

extern u8 C8_screen_scale;
static bool highres_mode = false;
extern u32 C8_screen_size;

static inline void C8_ToggleHighresMode( void ) {
		highres_mode = !highres_mode;
		if ( highres_mode ) { // Could be problematic
				C8_screen_scale /= 4;
		} else {
				C8_screen_scale *= 4;
		}
}

extern u8 C8_gfx[SCREEN_WIDTH * SCREEN_HEIGHT];
extern u8 SC8_gfx[(SCREEN_WIDTH * 2) * (SCREEN_HEIGHT * 2)];

static inline void C8_ClearDisplay( void ) {
    memset( C8_gfx, 0, C8_screen_size );
}

#endif /* PUNCHLINE_C8_DISPLAY_H */
