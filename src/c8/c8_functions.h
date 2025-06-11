/* Copyright 2025 Punchline Software

*/

#ifndef PUNCHLINE_C8_FUNCTIONS_H
#define PUNCHLINE_C8_FUNCTIONS_H

void C8_Fetch( void );
void C8_Execute( void );
void C8_InitChip8Compliance( void );

// Function dispatch tables
extern void (*C8Table[16])();
extern void (*C8System[256])();
extern void (*C8Math[16])();
extern void (*C8Key[256])();
extern void (*C8Mem[256])();

#endif /* PUNCHLINE_C8_FUNCTIONS_H */
