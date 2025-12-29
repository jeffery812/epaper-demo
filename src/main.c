#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

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
	
	// display_print(dev, "Happy New Year! 2026", 10, 50);
	// display_print(dev, "250x128 Mode", 10, 70);

	if (display_set_font(dev, 2) == 0) {
		display_print(dev, "19:48", 10, 40);
	}
	
	display_print(dev, "2026", 10, 80);
	/* Draw a rectangle to prove graphics work */
	display_draw_rect(dev, 5, 95, 195, 25);

	/* 2. Flush internal RAM buffer to Display (Slow, SPI transaction) */
	display_flush(dev);

	LOG_INF("Done.");
	return 0;
}
