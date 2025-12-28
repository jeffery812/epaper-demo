#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/logging/log.h>

#include "epd_graphics.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* --- Main Application --- */

int main(void)
{
	const struct device *dev = device_get_binding(CUSTOM_EPD_LABEL);

	LOG_INF("Zephyr CFB E-Paper Test");

	if (!device_is_ready(dev)) {
		LOG_ERR("Display device not ready");
		return 0;
	}

	if (display_set_pixel_format(dev, PIXEL_FORMAT_MONO10) != 0) {
		LOG_ERR("Failed to set required pixel format");
		return 0;
	}

	if (cfb_framebuffer_init(dev)) {
		LOG_ERR("Framebuffer initialization failed!");
		return 0;
	}

	cfb_framebuffer_clear(dev, false);

	/* Use default font (index 0) */
	if (cfb_framebuffer_set_font(dev, 0)) {
		LOG_WRN("Could not set font, CFB might not have fonts enabled in config");
	}

	LOG_INF("Drawing text with CFB...");
	
	/* 1. Draw to internal RAM buffer (Fast, no SPI transaction) */
	cfb_print(dev, "21:41", 10, 50);
	cfb_print(dev, "250x128 Mode", 10, 70);
	
	/* Draw a rectangle to prove graphics work */
	struct cfb_position start = {5, 40};
	struct cfb_position end = {200, 100};
	cfb_draw_rect(dev, &start, &end);

	/* Invert framebuffer to get black text on white background */
	cfb_framebuffer_invert(dev);

	/* 2. Flush internal RAM buffer to Display (Slow, SPI transaction) */
	LOG_INF("Finalizing...");
	cfb_framebuffer_finalize(dev);

	LOG_INF("Done.");
	return 0;
}
