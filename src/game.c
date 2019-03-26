#include <stdlib.h>
#include "os.h"
#include "game.h"
#include "demo.h"

#define MAX_TIME -1UL
#define FRAMES_PER_SECOND 40
// Frame duration in miliseconds.
#define FRAME_TIME (1000 / FRAMES_PER_SECOND)	// 25 ms. per frame -> 40 frames per second.
// Points per line. Double, triple and cuadriple line gives more points.
#define LINE_POINTS 10
// Time to wait between finished and demo states (measured in frame's duration multiples).
#define FINISHED_TIMEOUT (15000 / FRAME_TIME)

// (0, 0) are the piece's center. Rotations are based on this point.
piece_t PIECES[MAX_PIECES] = {
	{PT_PIPE, {{0, -1}, {0, 0}, {0, 1}, {0, 2}}, 1},		// Pipe
	{PT_RIGHTL, {{0, -1}, {0, 0}, {0, 1}, {1, 1}}, 1},   	// Right L
	{PT_LEFTL, {{1, -1}, {0, -1}, {0, 0}, {0, 1}}, 2},		// Left L
	{PT_SQUARE, {{0, 0}, {0, 1}, {1, 0}, {1, 1}}, 0},		// Square
	{PT_RIGHTS, {{-1, -1}, {-1, 0}, {0, 0}, {0, 1}}, 2},	// Right S
	{PT_LEFTS, {{0, -1}, {0, 0}, {-1, 0}, {-1, 1}}, 1},		// Left S
	{PT_T, {{0, -1}, {0, 0}, {0, 1}, {1, 0}}, 1}			// T
};

level_t LEVELS[] = {
	{40, 5}, {38, 8}, {36, 10}, {34, 12}, {32, 15}, {30, 18}, {28, 21},
	{26, 25}, {24, 28}, {22, 31}, {20, 35}, {18, 38}, {17, 42}, {16, 45},
	{15, 48}, {14, 50}, {13, 55}, {12, 60}, {11, 65}, {10, 70}
};

// Point multiplicator per line count.
unsigned int POINTS[4] = {1, 3, 9, 27};

gamestate_t State;

pos_t* GM_Rotate (pos_t* pos) {
	char t;

	t = pos->row;
	pos->row =- pos->col;
	pos->col = t;

	return pos;
}

enum piecetype_t GM_GeneratePiece () {
	return ((enum piecetype_t) (rand() % MAX_PIECES + 1));
}

void GM_InitBoard () {
	unsigned char r, c;

	for(r = 0; r < BOARD_ROWS; r++) {
		for(c = 0; c < BOARD_COLS; c++) {
			GM_SetBoardCell (r, c, PT_NONE);
		}
	}
}

void GM_InitStats () {
	unsigned char i;

	for (i = 0; i < MAX_PIECES; i++) {
		State.stats[i] = 0;
	}
}

piece_t* GM_CopyPiece (piece_t* in, piece_t* out) {
	unsigned char i;

	out->center = in->center;
	out->piecetype = in->piecetype;

	for (i = 0; i < PIECE_SQUARES; i++) {
		out->squares[i].row = in->squares[i].row;
		out->squares[i].col = in->squares[i].col;
	}

	return out;
}

void GM_CopyPos (pos_t* in, pos_t* out) {
	out->row = in->row;
	out->col = in->col;
}

void GM_Swap(pos_t* a, pos_t* b) {
	pos_t t;

	GM_CopyPos (b, &t);
	GM_CopyPos (a, b);
	GM_CopyPos (&t, a);
}

void GM_OrderSquaresByRow (piece_t* piece) {
	unsigned char i, j;

	for (i = 0; i < PIECE_SQUARES; i++) {
		for (j = PIECE_SQUARES - 1; j > i; j--) {
			if (piece->squares[j-1].row > piece->squares[j].row) {
				GM_Swap (&piece->squares[j - 1], &piece->squares[j]);
			}
		}
	}
}

void GM_GetPieceDims (piece_t* piece, unsigned char* width, unsigned char* height) {
	unsigned char i;
	char rmin, rmax, cmin, cmax;

	for (i = 0, rmin = 127, rmax = -127, cmin = 127, cmax = -127; i < PIECE_SQUARES; i++) {
		if (piece->squares[i].row < rmin) {
			rmin = piece->squares[i].row;
		}

		if (piece->squares[i].row > rmax) {
			rmax = piece->squares[i].row;
		}

		if (piece->squares[i].col < cmin) {
			cmin = piece->squares[i].col;
		}

		if (piece->squares[i].col > cmax) {
			cmax = piece->squares[i].col;
		}
	}

	*width = abs(cmax - cmin + 1);
	*height = abs(rmax - rmin + 1);
}

void GM_GetMinRowCol (piece_t* piece, char* rmin, char* cmin) {
	unsigned char i;

	for (i = 0, *rmin = 127, *cmin = 127; i < PIECE_SQUARES; i++) {
		if (piece->squares[i].row < *rmin) {
			*rmin = piece->squares[i].row;
		}

		if (piece->squares[i].col < *cmin) {
			*cmin = piece->squares[i].col;
		}
	}
}

void GM_GetMinMaxRow (piece_t* piece, char* min, char* max) {
	unsigned char i;

	for (i = 0, *min = 127, *max = -127; i < PIECE_SQUARES; i++) {
		if (piece->squares[i].row < *min) {
			*min = piece->squares[i].row;
		}

		if (piece->squares[i].row > *max) {
			*max = piece->squares[i].row;
		}
	}
}

void GM_GetMinMaxCol (piece_t* piece, char* min, char* max) {
	unsigned char i;

	for (i = 0, *min = 127, *max = -127; i < PIECE_SQUARES; i++) {
		if (piece->squares[i].col < *min) {
			*min = piece->squares[i].col;
		}

		if (piece->squares[i].col > *max) {
			*max = piece->squares[i].col;
		}
	}
}

char GM_GetMaxRow (piece_t* piece) {
	unsigned char i;
	char m;

	for (i = 0, m = -127; i < PIECE_SQUARES; i++) {
		if (piece->squares[i].row > m) {
			m = piece->squares[i].row;
		}
	}

	return m;
}

char GM_GetMinRow (piece_t* piece) {
	unsigned char i;
	char m;

	for (i = 0, m = 127; i < PIECE_SQUARES; i++) {
		if (piece->squares[i].row < m) {
			m = piece->squares[i].row;
		}
	}

	return m;
}

bool_t GM_IsCollision (piece_t* piece) {
	unsigned char i;

	for (i = 0; i < PIECE_SQUARES; i++) {
		if (GM_GetBoardCell (piece->squares[i].row, piece->squares[i].col) != PT_NONE) {
			return TRUE;
		}
	}

	return FALSE;
}

void GM_IsFullCollision (piece_t* piece, bool_t* isbottom, bool_t* iscoll) {
	unsigned char i;

	*isbottom = FALSE;
	*iscoll = FALSE;
	// Al four piece's squares must to be tested.
	for (i = 0; i < PIECE_SQUARES; i++) {
		if (piece->squares[i].row == 0) {
			*isbottom = TRUE;
		}

		if (!*isbottom) {
			if (GM_GetBoardCell (piece->squares[i].row - 1, piece->squares[i].col) != PT_NONE) {
				*iscoll = TRUE;
			}
		}
	}
}

void GM_AdjustBottomCollision (piece_t* piece, char* rowdelta) {
	unsigned char i;
	char m;

	m = GM_GetMinRow (piece);
	// "rowdelta" always < 0.
	if (abs (*rowdelta) > m) {
		*rowdelta = -m;
	}

	if (*rowdelta != 0) {
		for (i = 0; i < PIECE_SQUARES; i++) {
			if (GM_GetBoardCell (piece->squares[i].row + *rowdelta, piece->squares[i].col) != PT_NONE) {
				*rowdelta = 0;
				return;
			}
		}
	}
}

void GM_AdjustSideCollision (piece_t* piece, char* coldelta) {
	unsigned char i;

	for (i = 0; i < PIECE_SQUARES; i++) {
  		if (!((piece->squares[i].col + *coldelta < BOARD_COLS) && (piece->squares[i].col + *coldelta >= 0)) ||
				(GM_GetBoardCell (piece->squares[i].row, piece->squares[i].col + *coldelta) != PT_NONE)) {
			*coldelta = 0;
		}
	}
}

void GM_ShiftPiece (piece_t* piece, char rowdelta, char coldelta) {
	unsigned char i;

	for(i = 0; i < PIECE_SQUARES; i++) {
		piece->squares[i].row += rowdelta;
		piece->squares[i].col += coldelta;
	}
}

void GM_RotatePiece (piece_t* piece) {
	unsigned char i;

	for (i = 0; i < PIECE_SQUARES; i++) {
		GM_Rotate(&piece->squares[i]);
	}
}

void GM_DropPiece (piece_t* piece, char* rowdelta) {
	unsigned int i;
	char min;

	min = GM_GetMinRow (piece);

	if (State.toprow < 0) {
		*rowdelta =- min;
	} else {
		// "rowdelta" always negative.
		*rowdelta = State.toprow - min;
		for (;;(*rowdelta)--) {
			for (i = 0; i < PIECE_SQUARES; i++) {
				if ((piece->squares[i].row + *rowdelta < 0) ||
						(GM_GetBoardCell (piece->squares[i].row + *rowdelta, piece->squares[i].col) != PT_NONE)) {
					(*rowdelta)++;
					return;
				}
			}
		}
	}
}

void GM_InitPiece (piece_t* piece, enum piecetype_t piecetype) {
	unsigned char i;
	char row, col;

	piece->piecetype = piecetype;
	piece->center = PIECES[(unsigned int) (piece->piecetype - 1)].center;
	row = BOARD_ROWS - 1;
	col = (BOARD_COLS - 1) / 2;

	for (i = 0; i < PIECE_SQUARES; i++) {
		piece->squares[i].row = row + PIECES[(unsigned int) (piece->piecetype - 1)].squares[i].row;
		piece->squares[i].col = col + PIECES[(unsigned int) (piece->piecetype - 1)].squares[i].col;
	}
}

void GM_UpdatePiecePos () {
	piece_t p;
	char dr, dc;
	char min, max;

	dr = 0;
	dc = 0;
	switch (State.action) {
		case A_LEFT:
			dc = -1;
			break;
		case A_RIGHT:
			dc = 1;
			break;
		case A_UP:
			// Square do not rotate.
			if (State.piece.piecetype != PT_SQUARE) {
				GM_CopyPiece (&State.piece, &p);
				// Get the piece to the coordinates system origin.
				GM_ShiftPiece (&p, -State.piece.squares[State.piece.center].row, -State.piece.squares[State.piece.center].col);
				// Rotate it.
				GM_RotatePiece (&p);
				// Get the piece to the original position.
				GM_ShiftPiece (&p, State.piece.squares[State.piece.center].row, State.piece.squares[State.piece.center].col);
				// Now check collision.
				if (!GM_IsCollision (&p)) {
					GM_GetMinMaxCol (&p, &min, &max);

					if (min < 0) {
						GM_ShiftPiece (&p, 0, abs(min));
					}

					if (max >= BOARD_COLS) {
						GM_ShiftPiece (&p, 0, -(max-BOARD_COLS + 1));
					}

					GM_CopyPiece (&p, &State.piece);
				}
			}
			break;
		// Faster drop.
		case A_DOWN:
			dr = -1;
			GM_AdjustBottomCollision (&State.piece, &dr);
			break;
		case A_DROP:
			GM_DropPiece (&State.piece, &dr);
			break;
	}

	if (dc != 0) {
		GM_AdjustSideCollision (&State.piece, &dc);
	}

	if ((dr != 0) || (dc != 0)) {
		GM_ShiftPiece (&State.piece, dr, dc);
	}
}

void GM_GetLines (char* linerows, unsigned char* count) {
	char p[PIECE_SQUARES], r, c;
	unsigned char i, j;

	GM_OrderSquaresByRow (&State.piece);
	p[0] = State.piece.squares[0].row;

	for (i = 1, j = 0; i < PIECE_SQUARES; i++) {
		// We get non-repeated rows with posible completed lines.
		if (p[j] != State.piece.squares[i].row) {
			p[++j] = State.piece.squares[i].row;
		}
	}

	// Search for complete lines.
	for (r = p[0], i = 0; r <= p[0] + j; r++) {
		for (c = 0; c < BOARD_COLS; c++) {
			if (GM_GetBoardCell (r, c) == PT_NONE) {
				break;
			}
		}

		if (c == BOARD_COLS) {
			linerows[i++] = r;
		}
	}

	*count = i;
}

bool_t GM_IsLine (char* linerows, unsigned char* count) {
	GM_GetLines (linerows, count);

	if (*count == 0) {
		return FALSE;
	}

	return TRUE;
}

void GM_ClearLines (char* linerows, unsigned char count) {
	unsigned char i;
	char r, c;

	for (i = 0, r = linerows[i]; i < count; i++) {
		for (c = 0; c < BOARD_COLS; c++) {
			GM_SetBoardCell (r, c, PT_NONE);
		}
	}
}

void GM_DropLines (char from, char to) {
	char c;

	for (; from < BOARD_ROWS; from++, to++) {
		for (c = 0; c < BOARD_COLS; c++) {
			GM_SetBoardCell (to, c, GM_GetBoardCell (from, c));
		}
	}
}

void GM_Calculate(unsigned char count) {
	State.lines -= count;
	State.score += POINTS[count - 1] * LINE_POINTS;

	if (State.lines <= 0) {
		State.levelindex++;
		State.lines = LEVELS[State.levelindex].lines;
		State.frameacc = 0;
		State.level = State.levelindex + 1;
	}
}

void GM_BoundToBoard (piece_t* piece, char* toprow) {
	unsigned char i;
	char m;

	for (i = 0, m = -127; i < PIECE_SQUARES; i++) {
		if (piece->squares[i].row > m) {
			m = piece->squares[i].row;
		}

		GM_SetBoardCell (piece->squares[i].row, piece->squares[i].col, piece->piecetype);
	}

	if (m > *toprow) {
		*toprow = m;
	}
}

void GM_ExecEvent () {
	switch (State.currentevent->name) {
		case EV_ACTION:
			break;
		case EV_NEW_PIECE:
			GM_InitPiece (&State.piece, ((evnew_t*)(State.currentevent->extra))->piece);
			State.nextpiece = ((evnew_t*)(State.currentevent->extra))->nextpiece;
			break;
	}
}

void GM_Pause () {
	if (State.state == S_PAUSED) {
		State.state = S_PLAYING;
	} else {
		State.state = S_PAUSED;
	}
}

void GM_Init (enum state_t state) {
	srand (OS_GetTick ());
	GM_InitStats ();
	GM_InitBoard ();
	D_LoadEvents ();
	State.currentevent = D_GetFirstEvent ();

	// Generate pieces only in playing mode. In demo mode, pieces are generated by events.
	if (state == S_PLAYING) {
		State.nextpiece = GM_GeneratePiece ();
		GM_InitPiece (&State.piece, GM_GeneratePiece ());
	}

	State.state = state;
	State.score = 0;
	State.levelindex = 0;
	State.level = State.levelindex + 1;
	State.lines = LEVELS[State.levelindex].lines;
	State.toprow = -1;
	State.frameacc = 0;
	State.eventlengthcount = 0;
}

void GM_StartGame () {
	if (State.state != S_PLAYING) {
		GM_Init (S_PLAYING);
	}
}

void GM_SetKey (int key) {
	switch (key) {
		case KEY_LEFT:
			State.action = A_LEFT;
			break;
		case KEY_RIGHT:
			State.action = A_RIGHT;
			break;
		case KEY_UP:
			State.action = A_UP;
			break;
		case KEY_DOWN:
			State.action = A_DOWN;
			break;
		case KEY_SPACE:
			State.action = A_DROP;
			break;
		case KEY_S:
			GM_StartGame ();
			break;
		case KEY_P:
			GM_Pause ();
			break;
	}
}

void GM_Update () {
	char linerows[PIECE_SQUARES];
	unsigned char count;
	bool_t isbottom, iscoll;

	if (State.state == S_PAUSED) {
		return;
	}

	if (State.state == S_FINISHED) {
		// There is some time the legend "GAME OVER" and the stats are shown.
		// Expired this time (and no new game is started), the game comes back
		// to demo mode.
		if (State.frameacc == FINISHED_TIMEOUT) {
			GM_Init (S_DEMO);
			State.frameacc = 0;
		}

		return;
	}

	if (State.state == S_DEMO) {
		if (State.frameacc == LEVELS[State.levelindex].framecount) {
			State.eventlengthcount++;
			State.frameacc = 0;
		}

		if (State.eventlengthcount == State.currentevent->length) {
			State.currentevent = State.currentevent->next;
			State.eventlengthcount = 0;
			// If no events show stats for demo mode.
			if (State.currentevent->next == NULL) {
				GM_Init (S_FINISHED);
			} else {
				GM_ExecEvent ();
			}
		}

		return;
	}

	if (State.state == S_PLAYING && State.action != A_NONE) { 
		GM_UpdatePiecePos ();
		State.action = A_NONE;
	}

	// Check if some piece gets the board's top, bottom or collisioned with other squares.
	GM_IsFullCollision (&State.piece, &isbottom, &iscoll);

	// Check for posible lines if there is a collision.
	if (isbottom || iscoll) {
		// Bound piece to board before checking if there is a new line/s.
		GM_BoundToBoard (&State.piece, &State.toprow);
		// Add to stats.
		State.stats[(unsigned int) (State.piece.piecetype - 1)]++;

		if (State.toprow >= BOARD_ROWS) {
			State.state = S_FINISHED;
			State.frameacc = 0;
		} else {
			if (GM_IsLine (linerows, &count)) {
				GM_ClearLines (linerows, count);
				// Lower row is the lower index, the inverse from higher row.
				GM_DropLines (linerows[count - 1] + 1, linerows[0]);
				// Make adjustments and check end of level.
				GM_Calculate (count);
				// Adjust top row.
				State.toprow -= count;
			};
			// If state is S_DEMO pieces are generated by events.
			if (State.state == S_PLAYING) {
				// Make the old next piece the current piece.
				GM_InitPiece (&State.piece, State.nextpiece);
				// Generate new next piece.
				State.nextpiece = GM_GeneratePiece ();
			}
		}
	} else if (State.frameacc >= LEVELS[State.levelindex].framecount) {
		GM_ShiftPiece (&State.piece, -1, 0);
		State.frameacc = 0;
	}
}

void GM_SetBoardCell (char row, char col, enum piecetype_t value) {
	State.board[(unsigned char) row][(unsigned char) col] = value;
}

enum piecetype_t GM_GetBoardCell (char row, char col) {
	return (State.board[(unsigned char) row][(unsigned char) col]);
}

void GM_GetPieceTemplate (piece_t* piece, enum piecetype_t piecetype, unsigned char* w, unsigned char* h) {
	char rmin, cmin;

	GM_CopyPiece (&PIECES[(unsigned int) (piecetype - 1)], piece);
	GM_GetMinRowCol (piece, &rmin, &cmin);
	GM_GetPieceDims (piece, w, h);
	GM_ShiftPiece (piece, -rmin, -cmin);
}

unsigned int GM_GetMaxStat () {
	unsigned int i;
	int m;

	for (i = 0, m = -1; i < MAX_PIECES; i++) {
		if (State.stats[i] > m) {
			m = State.stats[i];
		}
	}

	return m;
}

void GM_Main () {
	unsigned long starttime, endtime, elapsed;

	OS_Init ();
	R_Init ();
	//GM_Init (S_DEMO);
	GM_Init (S_PLAYING);

	while (1) {
		starttime = OS_GetTick ();

		OS_ProcessEvents ();
		GM_Update ();
		R_Render ();
		OS_Render ();

		endtime = OS_GetTick ();

		// Check wrap-around.
		if (endtime < starttime) {
			elapsed = MAX_TIME - starttime + endtime;
		} else {
			elapsed = endtime - starttime;
		}

		// Elapsed time in microseconds. Convert to miliseconds.
		OS_Sleep (FRAME_TIME - elapsed / 1000);

		// Update elapsed frame count.
		State.frameacc++;
	}

	OS_Terminate ();
}
