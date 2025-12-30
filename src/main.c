#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/display/cfb.h>
#include <stdio.h>

#include "epd_graphics.h"
#include "display_lib.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* --- Main Application --- */

int main(void)
{
	const struct device *dev = device_get_binding(CUSTOM_EPD_LABEL);

	LOG_INF("Zephyr CFB E-Paper Test");

	if (display_lib_init(dev) != 0) {
		return 0;
	}

	LOG_INF("Drawing text with CFB...");

	int font_count = cfb_get_numof_fonts(dev);
	int best_idx = -1;
	uint8_t best_w = 0;
	uint8_t best_h = 0;

	for (int i = 0; i < font_count; i++) {
		uint8_t fw = 0;
		uint8_t fh = 0;

		if (cfb_get_font_size(dev, i, &fw, &fh) == 0) {
			if (fh > best_h) {
				best_idx = i;
				best_w = fw;
				best_h = fh;
			}
		}
	}

	if (best_idx < 0 || display_set_font(dev, best_idx) != 0) {
		LOG_WRN("No usable CFB fonts found");
		return 0;
	}

	LOG_INF("Using font index %d (%ux%u)", best_idx, best_w, best_h);

	// int seconds = (12 * 3600) + (34 * 60);
	int seconds = 0;
	int duration_in_seconds = 60 * 60; // 60 minutes
	while (1) {
		char time_str[6];
		int hours = (seconds / 3600) % 24;
		int minutes = (seconds / 60) % 60;

		snprintf(time_str, sizeof(time_str), "%02d:%02d", hours, minutes);

		cfb_framebuffer_clear(dev, false);
		/* Align to 8-pixel boundary to avoid partial glyph clipping */
		display_print(dev, time_str, 50, 40);
		display_flush(dev);

		seconds += duration_in_seconds;
		k_sleep(K_SECONDS(duration_in_seconds));
	}

	return 0;
}
