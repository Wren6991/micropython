#include "pico/types.h"
#include "dvi_stdout.h"

static void scroll() {
	// Note memcpy would be a bad idea here because the regions overlap
	for (uint y = 0; y < DVI_CHAR_ROWS - 1; ++y) {
		for (uint x = 0; x < DVI_CHAR_COLS; ++x) {
			dvi_charbuf[DVI_CHAR_COLS * y + x] = dvi_charbuf[DVI_CHAR_COLS * (y + 1) + x];
		}
	}
	for (uint x = 0; x < DVI_CHAR_COLS; ++x) {
		dvi_charbuf[DVI_CHAR_COLS * (DVI_CHAR_ROWS - 1) + x] = ' ';
	}
}

typedef enum {
	vt100_print,
	vt100_escape0,
	vt100_escape1
} vt100_state_t;

#define VT100_ESCAPE_CODE 0x1b
#define VT100_BACKSPACE_CODE 0x08

// well this has been an interesting trip to google
static void dvi_putchar(char c) {
	static uint col = 0;
	static vt100_state_t state = vt100_print;
	static uint cmd_decimal_accum = 0;
	char *final_row = &dvi_charbuf[DVI_CHAR_COLS * (DVI_CHAR_ROWS - 1)];
	switch (state) {
		case vt100_print: {
			if (c == VT100_ESCAPE_CODE) {
				state = vt100_escape0;
			}
			else if (c == VT100_BACKSPACE_CODE) {
				if (col > 0)
					--col;
			}
			else if (c == '\r') {
				col = 0;
			}
			else if (c == '\n') {
				scroll();
			}
			else if (c >= ' ' && c <= '~') {
				// Printable
				final_row[col++] = c;
				if (col >= DVI_CHAR_COLS) {
					scroll();
					col = 0;
				}
			}
			break;
		}
		case vt100_escape0: {
			if (c == '[') {
				state = vt100_escape1;
				cmd_decimal_accum = 0;
			}
			else {
				state = vt100_print;
			}
			break;
		}
		case vt100_escape1: {
			if (c == 'K') {
				state = vt100_print;
				for (uint i = col; i < DVI_CHAR_COLS; ++i) {
					final_row[i] = ' ';
				}
			}
			else if (c >= '0' && c <= '9') {
				cmd_decimal_accum = cmd_decimal_accum * 10u + (uint)(c - '0');
			}
			else if (c == 'D') {
				state = vt100_print;
				if (cmd_decimal_accum >= col)
					col = 0;
				else
					col -= cmd_decimal_accum;
			}
			else {
				state = vt100_print;
			}
			break;
		}
	}
}

// For use by mphalport.c
void dvi_write_strn(const char *str, uint len) {
	for (uint i = 0; i < len; ++i)
		dvi_putchar(str[i]);
}