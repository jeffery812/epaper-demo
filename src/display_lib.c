/* src/display_lib.c */
#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/logging/log.h>
#include "display_lib.h"

LOG_MODULE_REGISTER(display_lib, LOG_LEVEL_INF);

/**
 * @brief Initialize the display library.
 *
 * @details This function must be called before any other display_lib functions.
 * It performs the following steps:
 * - Verifies that the display device is ready.
 * - Sets the pixel format to monochrome.
 * - Initializes the Character Framebuffer (CFB) subsystem.
 * - Clears the framebuffer.
 * - Sets the default font.
 *
 * @usage
 * @code
 * #include "display_lib.h"
 * #include "epd_graphics.h" // For CUSTOM_EPD_LABEL
 *
 * void main(void)
 * {
 *     const struct device *dev = device_get_binding(CUSTOM_EPD_LABEL);
 *     if (display_lib_init(dev) != 0) {
 *         // Handle initialization failure
 *         return;
 *     }
 *
 *     display_print(dev, "Ready!", 0, 0);
 *     display_flush(dev);
 * }
 * @endcode
 *
 * @param dev Pointer to the display device instance to initialize.
 *
 * @return 0 on success.
 * @retval -ENODEV If the display device is not ready.
 * @retval -EIO If setting pixel format or initializing the framebuffer fails.
 */
int display_lib_init(const struct device *dev)
{
	if (!device_is_ready(dev)) {
		LOG_ERR("Display device not ready");
		return -ENODEV;
	}

	if (display_set_pixel_format(dev, PIXEL_FORMAT_MONO10) != 0) {
		LOG_ERR("Failed to set required pixel format");
		return -EIO;
	}

	if (cfb_framebuffer_init(dev)) {
		LOG_ERR("Framebuffer initialization failed!");
		return -EIO;
	}

	cfb_framebuffer_clear(dev, false);

	/* Use default font (index 0) */
	if (cfb_framebuffer_set_font(dev, 0)) {
		LOG_WRN("Could not set font, CFB might not have fonts enabled in config");
	}

	return 0;
}

void display_print(const struct device *dev, const char *str, uint16_t x, uint16_t y)
{
	cfb_print(dev, str, x, y);
}

int display_set_font(const struct device *dev, uint8_t font_idx)
{
	return cfb_framebuffer_set_font(dev, font_idx);
}

void display_draw_rect(const struct device *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	struct cfb_position start = {x, y};
	struct cfb_position end = {x + w, y + h};
	cfb_draw_rect(dev, &start, &end);
}

void display_flush(const struct device *dev)
{
	/* Invert framebuffer to get black text on white background */
	cfb_framebuffer_invert(dev);
	
	LOG_INF("Finalizing...");
	cfb_framebuffer_finalize(dev);
}
