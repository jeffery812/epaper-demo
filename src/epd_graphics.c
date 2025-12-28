/* src/epd_graphics.c */
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include "epd_driver.h"
#include "epd_graphics.h"

LOG_MODULE_REGISTER(epd_graphics, LOG_LEVEL_INF);

/* Logical Landscape Resolution */
#define LOGICAL_WIDTH  248 /* Multiple of 8, fits in 250 */
#define LOGICAL_HEIGHT 128

/* --- Zephyr Display Driver Wrapper --- */
/* This wrapper allows the CFB subsystem to use our manual EPD driver */

static int custom_epd_blanking_off(const struct device *dev)
{
	return 0;
}

static int custom_epd_blanking_on(const struct device *dev)
{
	return 0;
}

static uint8_t rotated_buffer[EPD_WIDTH_BYTES * EPD_HEIGHT];

static int custom_epd_write(const struct device *dev, const uint16_t x, const uint16_t y,
			    const struct display_buffer_descriptor *desc, const void *buf)
{
	const uint8_t *src = buf;
	uint8_t *dst = rotated_buffer;
	
	/* Clear destination buffer */
	memset(dst, 0, sizeof(rotated_buffer));

	/* 
	 * Rotate 90 degrees CW
	 * Logical (Landscape) -> Physical (Portrait)
	 * Logical X [0..247] -> Physical Y [249..2]
	 * Logical Y [0..127] -> Physical X [0..127]
	 */
	for (int ly = 0; ly < LOGICAL_HEIGHT; ly++) {
		for (int lx = 0; lx < LOGICAL_WIDTH; lx++) {
			/* Get pixel from Source (Logical) */
			if (src[(ly * (LOGICAL_WIDTH / 8)) + (lx / 8)] & (0x80 >> (lx % 8))) {
				/* Map to Destination (Physical) */
				int px = ly;
				int py = (EPD_HEIGHT - 1) - lx;

				dst[(py * EPD_WIDTH_BYTES) + (px / 8)] |= (0x80 >> (px % 8));
			}
		}
	}

	epd_display_framebuffer(rotated_buffer, sizeof(rotated_buffer));
	return 0;
}

static int custom_epd_read(const struct device *dev, const uint16_t x, const uint16_t y,
			   const struct display_buffer_descriptor *desc, void *buf)
{
	return -ENOTSUP;
}

static void custom_epd_get_capabilities(const struct device *dev,
					struct display_capabilities *caps)
{
	caps->x_resolution = LOGICAL_WIDTH;
	caps->y_resolution = LOGICAL_HEIGHT;
	caps->supported_pixel_formats = PIXEL_FORMAT_MONO10;
	caps->current_pixel_format = PIXEL_FORMAT_MONO10;
	caps->current_orientation = DISPLAY_ORIENTATION_NORMAL;
	caps->screen_info = SCREEN_INFO_MONO_MSB_FIRST; /* Standard horizontal mapping, MSB first */
}

static int custom_epd_set_pixel_format(const struct device *dev,
				       const enum display_pixel_format pf)
{
	if (pf == PIXEL_FORMAT_MONO10) {
		return 0;
	}
	return -ENOTSUP;
}

static const struct display_driver_api custom_epd_api = {
	.blanking_on = custom_epd_blanking_on,
	.blanking_off = custom_epd_blanking_off,
	.write = custom_epd_write,
	.read = custom_epd_read,
	.get_capabilities = custom_epd_get_capabilities,
	.set_pixel_format = custom_epd_set_pixel_format,
};

static int custom_epd_init(const struct device *dev)
{
	int err;

	/* Initialize hardware (SPI, GPIO) */
	err = epd_hardware_init();
	if (err) {
		return err;
	}

	/* Run the specific V4 initialization sequence */
	epd_init_v4();

	return 0;
}

/* Define the device instance */
DEVICE_DEFINE(custom_epd, CUSTOM_EPD_LABEL, &custom_epd_init, NULL, NULL, NULL,
	      POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY, &custom_epd_api);