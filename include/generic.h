#ifndef GENERIC_H
#define GENERIC_H

#ifndef TRUE
#define TRUE 0xFF
#endif

#ifndef FALSE
#define FALSE 0x00
#endif

enum piecetype_t {PT_NONE, PT_PIPE, PT_RIGHTL, PT_LEFTL, PT_SQUARE, PT_RIGHTS, PT_LEFTS, PT_T};

enum action_t {A_NONE, A_LEFT, A_RIGHT, A_UP, A_DOWN, A_DROP};

typedef unsigned char bool_t;

typedef struct pos_s {
  char row;
  char col;
} pos_t;

#endif
