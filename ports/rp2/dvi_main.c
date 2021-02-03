#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "dvi.h"
#include "tmds_encode.h"
#include "dvi_stdout.h"

#include "font_8x8.h"

// These have external linkage in the stdout shim and no, we don't synchronise the accesses :)
char dvi_charbuf[DVI_CHAR_ROWS * DVI_CHAR_COLS];

// Using PIO1 SM0,1,2 for DVI.  MicroPython can have PIO0. We do have a lovely
// hardware_claim library to handle this but it's getting late :)
static const struct dvi_serialiser_cfg picodvi_dvi_cfg = {
	.pio = pio1,
	.sm_tmds = {0, 1, 2},
	.pins_tmds = {10, 12, 14},
	.pins_clk = 8,
	.invert_diffpairs = true
};

struct dvi_inst dvi0;

static void __not_in_flash_func(prepare_scanline)(const char *chars, uint y) {
	static __attribute__((aligned(4))) uint8_t scanbuf[DVI_FRAME_WIDTH / 8];
	// First blit font into 1bpp scanline buffer, then encode scanbuf into tmdsbuf
	for (uint i = 0; i < DVI_CHAR_COLS; ++i) {
		scanbuf[i] = font_8x8[(chars[i] - FONT_FIRST_ASCII) + (y % FONT_CHAR_HEIGHT) * FONT_N_CHARS];
	}
	uint32_t *tmdsbuf;
	queue_remove_blocking(&dvi0.q_tmds_free, &tmdsbuf);
	tmds_encode_1bpp((const uint32_t*)scanbuf, tmdsbuf, DVI_FRAME_WIDTH);
	queue_add_blocking(&dvi0.q_tmds_valid, &tmdsbuf);
}

void __not_in_flash_func(dvi_main)() {
	for (int i = 0; i < DVI_CHAR_ROWS * DVI_CHAR_COLS; ++i)
		dvi_charbuf[i] = ' ';

	dvi0.timing = &DVI_TIMING;
	dvi0.ser_cfg = picodvi_dvi_cfg;
	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());
	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_1);
	dvi_start(&dvi0);

	while (true) {
		for (uint y = 0; y < DVI_FRAME_HEIGHT; ++y) {
			prepare_scanline(&dvi_charbuf[DVI_CHAR_COLS * (y / FONT_CHAR_HEIGHT)], y);
		}
	}

}
