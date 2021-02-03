#ifndef _DVI_STDOUT_H
#define _DVI_STDOUT_H

#include "dvi.h"

#define FONT_CHAR_WIDTH 8
#define FONT_CHAR_HEIGHT 8
#define FONT_N_CHARS 95
#define FONT_FIRST_ASCII 32

#define DVI_TIMING dvi_timing_640x480p_60hz
#define DVI_FRAME_WIDTH 640
#define DVI_FRAME_HEIGHT 480

#define DVI_CHAR_COLS (DVI_FRAME_WIDTH / FONT_CHAR_WIDTH)
#define DVI_CHAR_ROWS (DVI_FRAME_HEIGHT / FONT_CHAR_HEIGHT)

extern char dvi_charbuf[];

void dvi_write_strn(const char *str, uint len);

void dvi_main(void);

#endif