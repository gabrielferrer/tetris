#ifndef GAME_H
#define GAME_H

#include "generic.h"
#include "demo.h"
#include "render.h"

// Squares by piece.
#define PIECE_SQUARES 4
// Types of distinct pieces.
#define MAX_PIECES 7

// Define game keys
#define KEY_NONE 0
#define KEY_LEFT 1
#define KEY_RIGHT 2
#define KEY_UP 3
#define KEY_DOWN 4
#define KEY_SPACE 5
#define KEY_S 6
#define KEY_P 7

enum state_t {S_DEMO, S_PLAYING, S_PAUSED, S_FINISHED};

typedef struct level_s {
	unsigned int framecount;	// How many frames to wait between game updates at this level.
	unsigned int lines;			// Lines to complete at this level.
} level_t;

typedef struct piece_s {
	enum piecetype_t piecetype;
	pos_t squares[PIECE_SQUARES];
	unsigned char center;
} piece_t;

typedef struct gamestate_s {
	enum state_t state;								// Game state (playing, paused, finished, demo).
	unsigned int score;								// Score.
	unsigned char level;							// Level number.
	unsigned char levelindex;						// Current level index.
	char lines;										// Lines left can be a negative number.
	char toprow;									// The max row occupied by some piece's square.
	enum piecetype_t nextpiece;						// Next piece to come.
	unsigned int frameacc;							// Frame accumulator. Every frame this adds one.
	event_t* currentevent;							// The current event (demo mode).
	unsigned int eventlengthcount;					// Count for event's length.
	piece_t piece;									// Current piece parts.
	enum piecetype_t board[BOARD_ROWS][BOARD_COLS];	// Just the board.
	int stats[MAX_PIECES];							// Stats.
	enum action_t action;							// Current action.
} gamestate_t;

void GM_Main ();
void GM_SetKey (int key);
void GM_SetBoardCell (char row, char col, enum piecetype_t value);
enum piecetype_t GM_GetBoardCell (char row, char col);
void GM_GetPieceTemplate (piece_t* piece, enum piecetype_t piecetype, unsigned char* w, unsigned char* h);
unsigned int GM_GetMaxStat ();

#endif
