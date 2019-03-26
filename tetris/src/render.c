#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "game.h"
#include "render.h"
#include "bitmap.h"

#define FONT_PIXEL_PER_SQUARE 16
// Keep the extra with a multiple of 4.
#define STATS_HEIGHT 100
#define STATS_BAR_WIDTH 30
#define STATS_BAR_GAP 5
#define GRADIENT_SHIFT 100.0f
#define RESOURCE_PATH "./"

char* MSGS[] = {"SCORE %d", "LEVEL %d", "LINES LEFT %d", "NEXT", "GAME OVER", "STATS", "PAUSED"};

unsigned int PIECECOLORS[] = {0x0000f3ef, 0x00f7a200, 0x000000f7, 0x00f7f300, 0x0000f300, 0x00f70000, 0x009c00ef};

extern gamestate_t State;
bitmap_t* Font;
extern gamestate_t State;
unsigned int* Scene;		// Rendered scene.

/*
	Returns a piece color.

	[IN]
		piecetype: piece's type.

	[RETURN]
		piece's color.
*/
unsigned int R_GetColor (enum piecetype_t piecetype) {
	return PIECECOLORS[(unsigned int) (piecetype - 1)];
}

/*
	Checks if the channel is into range [0..255].

	[IN]
		channel: unsigned char channel to check (as int).

	[RETURN]
		Channel into range [0..255].
*/
unsigned int R_CheckRange (int channel) {
	if (channel > 255) {
		return 255;
	}

	if (channel < 0) {
		return 0;
	}

	return (unsigned int) channel;
}

/*
	Adds two colors.

	[IN]
		color1: pointer to an int with format xxRRGGBB (as unsigned char).
		color2: idem color1.

	[RETURN]
		The two color added.
*/
unsigned int R_ColorAdd (unsigned char* color1, unsigned char* color2) {
	return R_CheckRange (color1[1] + color2[1]) << 16 | R_CheckRange (color1[2] + color2[2]) << 8 | R_CheckRange (color1[3] + color2[3]);
}

/*
	Multiplies a color by a constant.

	[IN]
		color: pointer to an int with format xxRRGGBB (as unsigned char).
		constant: constant to multiply.

	[RETURN]
		The color multiplied by the constant.
*/
unsigned int R_ColorConstMult (unsigned char* color, float constant) {
	return R_CheckRange ((float) color[1] * constant) << 16 | R_CheckRange ((float) color[2] * constant) << 8 |
		R_CheckRange ((float) color[3] * constant);
}

/*
	Adds a color by a constant.

	[IN]
		color: pointer to an int with format xxRRGGBB (as unsigned char).
		constant: constant to add.

	[RETURN]
		The color added to a constant.
*/
unsigned int R_ColorConstAdd (unsigned char* color, float constant) {
	return R_CheckRange ((float) color[1] + constant) << 16 | R_CheckRange ((float) color[2] + constant) << 8 |
		R_CheckRange ((float) color[3] + constant);
}

/*
	Draws a horizontal line.

	[IN]
		x: coordinate over the X axis.
		y: coordinate over the Y axis.
		length: line's length in pixels.
		color: line's color
*/
void R_DrawHorizontalLine (unsigned int x, unsigned int y, unsigned int length, unsigned int color) {
	unsigned int i, base;

	base = y * WINDOW_WIDTH + x;
	for (i = 0; i < length; i++) {
		Scene[base + i] = color;
	}
}

/*
	Draws a vertical gradient.

	[IN]
		ystart: coordinate over the Y axis where the gradient begins.
		yend: coordinate over the Y axis where the gradient ends.
		width: gradient's width.
		cstart: gradient start color.
		cend: gradient end color.
*/
void R_DrawVerticalGradient (unsigned int ystart, unsigned int yend, unsigned int xstart, unsigned int width, unsigned int cstart, unsigned int cend) {
	unsigned int i, cr, cs, ce;
	float delta, d;

	delta = 1.0f / (yend - ystart + 1);
	for (i = ystart, d = 0.0f; i <= yend; i++, d += delta) {
		cs = R_ColorConstMult ((unsigned char*) &cstart, d);
		ce = R_ColorConstMult ((unsigned char*) &cend, 1.0f - d);
		cr = R_ColorAdd ((unsigned char*) &cs, (unsigned char*) &ce);
		R_DrawHorizontalLine (xstart, i, width, cr);
	}
}

/*
	Returns the statistics height in pixels.
*/
unsigned int R_GetStatHeight () {
	return STATS_HEIGHT;
}

/*
	Returns a string width in pixels.

	[IN]
		s: string who's width its returned.
*/
unsigned int R_StringWidth (char* s) {
	char *p;

	p = s;
	while(*++p)
		;

	return ((p - s) * FONT_PIXEL_PER_SQUARE);
}

/*
	Returns a string height in pixels.
*/
unsigned int R_StringHeight () {
	return FONT_PIXEL_PER_SQUARE;
}

/*
	Draws an area of a bitmap.

	[IN]
		bitmap: source bitmap.
		xd: coordinate over the X axis into destination buffer.
		yd: coordinate over the Y axis into destination buffer.
		xs: coordinate over the X axis into source bitmap.
		ys: coordinate over the Y axis into source bitmap.
		width: area to copy width.
		height: area to copy height.
*/
void R_DrawBmp (bitmap_t* bitmap, unsigned int xd, unsigned int yd, unsigned int xs, unsigned int ys, unsigned int width, unsigned int height) {
	unsigned int h, w, srcbase, destbase;

	for (h = 0; h < height; h++) {
		srcbase = (ys + h) * bitmap->width + xs;
		destbase = (yd + h) * WINDOW_WIDTH + xd;

		for (w = 0; w < width; w++) {
			if (bitmap->data[srcbase + w] != 0x00000000) {
				Scene[destbase + w] = bitmap->data[srcbase + w];
			}
		}
	}
}

/*
	Prints a char.

	[IN]
		c: char to print.
		x: coordinate over the X axis where the char is printed.
		y: coordinate over the Y axis where the char is printed.
*/
void R_PrintChar (char c, unsigned int x, unsigned int y) {
	if (c < 0x20 || c > 0x7E) {
		c = 0x7F;
	}

	R_DrawBmp (Font, x, y, (c - 0x20) % FONT_PIXEL_PER_SQUARE * FONT_PIXEL_PER_SQUARE, (c - 0x20) / FONT_PIXEL_PER_SQUARE * FONT_PIXEL_PER_SQUARE,
		FONT_PIXEL_PER_SQUARE, FONT_PIXEL_PER_SQUARE);
}

/*
	Prints a string.

	[IN]
		c: string to print.
		x: coordinate over the X axis where the string is printed.
		y: coordinate over the Y axis where the string is printed.
*/
void R_PrintString (char* s, unsigned int x, unsigned int y) {
	unsigned int offset;
	char* p;

	offset = 0;
	p = s;
	while (*p) {
		R_PrintChar (*p, x + offset, y);
		offset += FONT_PIXEL_PER_SQUARE;
		p++;
	}
}

/*
	Draws a rectangle with a gradient and a thin empty border.

	[IN]
		x: coordinate over the X axis where the rectangle begins.
		y: coordinate over the Y axis where the rectangle begins.
		width: rectangle's width.
		height: rectangle's height.
		color: rectangle average color.
*/
void R_DrawRect (unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color) {
	unsigned int cs, ce;

	cs = R_ColorConstAdd ((unsigned char*) &color, GRADIENT_SHIFT);
	ce = R_ColorConstAdd ((unsigned char*) &color, -GRADIENT_SHIFT);
	R_DrawVerticalGradient (y + 1, y + height - 2, x + 1, width - 2, cs, ce);
}

/*
	Prints a formated text.

	[IN]
		x: coordinate over the X axis where the text is printed.
		y: coordinate over the Y axis where the text is printed.
		format: text's format.
		...: params for format.
*/
void R_DrawFormatedText (unsigned int x, unsigned int y, const char* format, ...) {
#define BUFFERSIZE 20
	char s[BUFFERSIZE];
	va_list list;

	memset (s, 0, BUFFERSIZE);
	va_start (list, format);
	vsprintf (s, format, list);
	va_end (list);
	R_PrintString (s, x, y);
}

/*
	Draws the board.
*/
void R_DrawBoard () {
	char r, c;

	for (r = 0; r < BOARD_ROWS; r++) {
		for (c = 0; c < BOARD_COLS; c++) {
			if (GM_GetBoardCell (r, c) != PT_NONE) {
				R_DrawRect (
					c * PIXELS_PER_SQUARE,
					BOARD_START + (BOARD_ROWS - r - 1) * PIXELS_PER_SQUARE,
					PIXELS_PER_SQUARE,
					PIXELS_PER_SQUARE,
					R_GetColor (GM_GetBoardCell (r, c)));
			}
		}
	}
}

/*
	Draws a piece.

	[IN]
		piece: piece to draw.
*/
void R_DrawPiece (piece_t* piece) {
	unsigned char i;

	for (i = 0; i < PIECE_SQUARES; i++) {
		R_DrawRect (
			piece->squares[i].col * PIXELS_PER_SQUARE,
			BOARD_START + (BOARD_ROWS - piece->squares[i].row - 1) * PIXELS_PER_SQUARE,
			PIXELS_PER_SQUARE,
			PIXELS_PER_SQUARE,
			R_GetColor (piece->piecetype));
	}
}

/*
	Draws the small piece who appear at upper right of screen (the next piece).
*/
void R_DrawNextPiece () {
	piece_t piece;
	unsigned char i, w, h;

	GM_GetPieceTemplate (&piece, State.nextpiece, &w, &h);
	for (i = 0; i < PIECE_SQUARES; i++) {
		R_DrawRect (
			WINDOW_WIDTH - (w - piece.squares[i].col) * PIXELS_PER_SMALL_SQUARE,
			(h - piece.squares[i].row) * PIXELS_PER_SMALL_SQUARE,
			PIXELS_PER_SMALL_SQUARE,
			PIXELS_PER_SMALL_SQUARE,
			R_GetColor (piece.piecetype));
	}
}

/*
	Draws the statistics (centered horizontally).

	[IN]
		ystart: coordinate over the Y axis where the stats begins.
*/
void R_DrawStats (unsigned int ystart) {
	unsigned int i, x, max, color, barheight;

	x = (BOARD_WIDTH - (MAX_PIECES * STATS_BAR_WIDTH + STATS_BAR_GAP * (MAX_PIECES - 1))) / 2;
	max = GM_GetMaxStat ();

	if (max == 0) {
		return;
	}

	for (i = 0; i < MAX_PIECES; i++, x += STATS_BAR_WIDTH + STATS_BAR_GAP) {
		barheight = State.stats[i] * R_GetStatHeight () / max;
		R_DrawRect (x, ystart + max - barheight, STATS_BAR_WIDTH, barheight, PIECECOLORS[i]);
	}
}

/*
	Inits the render.
*/
void R_Init () {
	Font = BMP_LoadBitmap (RESOURCE_PATH, "font");
}

/*
	Renders the game.
*/
void R_Render () {
	unsigned int sh, y;

	memset (Scene, 0, BYTES_PER_LINE * WINDOW_HEIGHT);
	sh = R_StringHeight ();
	R_DrawFormatedText (0, 0, MSGS[0], State.score);
	R_DrawFormatedText (0, sh, MSGS[1], State.level);
	R_DrawFormatedText (0, 2 * sh, MSGS[2], State.lines);
	R_DrawFormatedText (BOARD_WIDTH - R_StringWidth (MSGS[3]), 0, MSGS[3]);

	if (State.state == S_PLAYING || State.state == S_DEMO) {
		// The next piece.
		R_DrawNextPiece ();
		// Draw the board.
		R_DrawBoard ();
		// Draw the current piece.
		R_DrawPiece (&State.piece);
	} else if (State.state == S_PAUSED) {
		R_DrawFormatedText ((BOARD_WIDTH - R_StringWidth (MSGS[6])) / 2, BOARD_START + (BOARD_HEIGHT - sh) / 2, MSGS[6]);
	} else if (State.state == S_FINISHED) {
		y = BOARD_START + (BOARD_HEIGHT - sh - sh - R_GetStatHeight ()) / 2;
		R_DrawStats (y);
		R_DrawFormatedText ((BOARD_WIDTH - R_StringWidth (MSGS[4])) / 2, y + R_GetStatHeight (), MSGS[4]);
		R_DrawFormatedText ((BOARD_WIDTH - R_StringWidth (MSGS[5])) / 2, y + R_GetStatHeight () + sh, MSGS[5]);
	}

	if (State.state == S_DEMO) {
		if (State.currentevent->messageindex >= 0) {
			R_DrawFormatedText (
				(BOARD_WIDTH - R_StringWidth (MSGS[State.currentevent->messageindex])) / 2,
				BOARD_START + (BOARD_HEIGHT - sh) / 2,
				MSGS[State.currentevent->messageindex]);
		}
	}
}

