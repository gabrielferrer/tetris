#include <stdlib.h>
#include <string.h>
#include "demo.h"

#define BUFFERSIZE 10

typedef struct stream_s {
	event_t* events;
	char buffer[BUFFERSIZE];
	unsigned int streamindex;
	bool_t eos;
} stream_t;

const char* EVSTREAM = "1,1,-1,rs,t:2,1,-1,d";

stream_t Stream;

void D_InitStream () {
	Stream.events = NULL;
	memset (Stream.buffer, 0, BUFFERSIZE);
	Stream.streamindex = 0;
	Stream.eos = FALSE;
}

char* D_GetField () {
	unsigned int bidx;

	bidx = 0;
	while (EVSTREAM[Stream.streamindex] != ',' && EVSTREAM[Stream.streamindex] != ':' && EVSTREAM[Stream.streamindex] != '\0') {
		Stream.buffer[bidx++] = EVSTREAM[Stream.streamindex++];
	}

	Stream.buffer[bidx] = '\0';
	Stream.eos = EVSTREAM[Stream.streamindex++] == '\0';

	return Stream.buffer;
}

unsigned int D_GetLength () {
	return atoi (D_GetField ());
}

enum eventname_t D_GetName () {
	D_GetField();

	if (strcmp (Stream.buffer, "n") == 0) {
		return EV_NEW_PIECE;
	}

	if (strcmp (Stream.buffer, "a") == 0) {
		return EV_ACTION;
	}

	return EV_UNKNOWN;
}

int D_GetMsgIndex () {
	return atoi (D_GetField ());
}

enum piecetype_t D_GetPiece () {
	D_GetField ();

	if (strcmp (Stream.buffer, "p") == 0) {
		return PT_PIPE;
	}

	if (strcmp (Stream.buffer, "rl") == 0) {
		return PT_RIGHTL;
	}

	if (strcmp (Stream.buffer, "ll") == 0) {
		return PT_LEFTL;
	}

	if (strcmp (Stream.buffer, "s") == 0) {
		return PT_SQUARE;
	}

	if (strcmp (Stream.buffer, "rs") == 0) {
		return PT_RIGHTS;
	}

	if (strcmp (Stream.buffer, "ls") == 0) {
		return PT_LEFTS;
	}

	if (strcmp (Stream.buffer, "t") == 0) {
		return PT_T;
	}

	return PT_NONE;
}

enum action_t D_GetAction () {
	D_GetField ();

	if (strcmp (Stream.buffer, "l") == 0) {
		return A_LEFT;
	}

	if (strcmp (Stream.buffer, "r") == 0) {
		return A_RIGHT;
	}

	if (strcmp (Stream.buffer, "u") == 0) {
		return A_UP;
	}

	if (strcmp (Stream.buffer, "d") == 0) {
		return A_DOWN;
	}

	if (strcmp (Stream.buffer, "p") == 0) {
		return A_DROP;
	}

	return A_NONE;
}

enum eventname_t D_GetEventName () {
	return (enum eventname_t) atoi (D_GetField ());
}

void D_GetEvCommonData (event_t* pev, enum eventname_t eventname) {
	pev->name = eventname;
	pev->length = D_GetLength ();
	pev->messageindex = D_GetMsgIndex ();
	pev->next = NULL;
}

void D_GetActionEvData (event_t* pev) {
	evaction_t* pevm;

	pevm = (evaction_t*) pev->extra;
	pevm->action = D_GetAction ();
}

event_t* D_ActionEvent (enum eventname_t eventname) {
	event_t* pev;

	if ((pev = (event_t*) malloc (sizeof (event_t))) == NULL) {
		return NULL;
	}

	if ((pev->extra = (evaction_t*) malloc (sizeof (evaction_t))) == NULL) {
		goto clean;
	}

	D_GetEvCommonData (pev, eventname);
	D_GetActionEvData (pev);

	return pev;

clean:
	free (pev);

	return NULL;
}

void D_GetNewPieceEvData (event_t* pev) {
	evnew_t* pevn;

	pevn = (evnew_t*) pev->extra;
	pevn->piece = D_GetPiece ();
	pevn->nextpiece = D_GetPiece ();
}

event_t* D_NewPieceEvent (enum eventname_t eventname) {
	event_t* pev;

	if ((pev = (event_t*) malloc (sizeof (event_t))) == NULL) {
		return NULL;
	}

	if ((pev->extra = (evnew_t*) malloc (sizeof (evnew_t))) == NULL) {
		goto clean;
	}

	D_GetEvCommonData (pev, eventname);
	D_GetNewPieceEvData (pev);

	return pev;

clean:
	free (pev);

	return NULL;
}

void D_LoadEvents () {
	enum eventname_t e;
	event_t* pev, * last;

	D_InitStream ();

	while (!Stream.eos) {
		e = D_GetEventName ();

		switch (e) {
			case EV_NEW_PIECE:
				pev = D_NewPieceEvent (e);
				break;
			case EV_ACTION:
				pev = D_ActionEvent (e);
				break;
			case EV_UNKNOWN:
				break;
		}

		if (pev != NULL) {
			if (Stream.events) {
				last->next = pev;
			} else {
				Stream.events = pev;
			}

			last = pev;
		}
	}
}

event_t *D_GetFirstEvent() {
	return Stream.events;
}
