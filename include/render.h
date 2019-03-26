#ifndef RENDER_H
#define RENDER_H

#define BYTES_PER_PIXEL sizeof (unsigned int)
// Board rows and columns.
#define BOARD_ROWS 20
#define BOARD_COLS 10
#define PIXELS_PER_SQUARE 30
#define PIXELS_PER_SMALL_SQUARE 20
#define BOARD_START 90
#define BOARD_WIDTH 300
#define BOARD_HEIGHT 600
#define WINDOW_WIDTH BOARD_WIDTH
#define WINDOW_HEIGHT 690
#define BYTES_PER_LINE WINDOW_WIDTH * BYTES_PER_PIXEL

void R_Init ();
void R_Render ();

#endif
