#ifndef DEMO_H
#define DEMO_H

#include "generic.h"

enum eventname_t {EV_UNKNOWN, EV_NEW_PIECE, EV_ACTION};

typedef struct evaction_s {
	enum action_t action;
} evaction_t;

typedef struct evnew_s {
	enum piecetype_t piece;
	enum piecetype_t nextpiece;
} evnew_t;

typedef struct event_s {
	unsigned int length;
	enum eventname_t name;
	int messageindex;
	void* extra;
	struct event_s* next;
} event_t;

void D_LoadEvents ();
event_t* D_GetFirstEvent();

#endif
