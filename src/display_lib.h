/* src/display_lib.h */
#ifndef DISPLAY_LIB_H
#define DISPLAY_LIB_H

#include <zephyr/device.h>
#include <stdint.h>

/**
 * @brief Initialize the display library.
 *
 * This function must be called before any other display_lib functions.
 * It performs the following steps:
 * - Verifies that the display device is ready.
 * - Sets the pixel format to monochrome.
 * - Initializes the Character Framebuffer (CFB) subsystem.
 * - Clears the framebuffer.
 * - Sets the default font.
 *
 * @param dev Pointer to the display device instance to initialize.
 *
 * @return 0 on success.
 * @retval -ENODEV If the display device is not ready.
 * @retval -EIO If setting pixel format or initializing the framebuffer fails.
 */
int display_lib_init(const struct device *dev);

/**
 * @brief Print text to the display buffer
 * @param dev Display device instance
 * @param str Text string to print
 * @param x X coordinate
 * @param y Y coordinate
 */
void display_print(const struct device *dev, const char *str, uint16_t x, uint16_t y);

/**
 * @brief Set the active CFB font by index
 * @param dev Display device instance
 * @param font_idx Font index as configured in Zephyr
 *
 * @return 0 on success.
 * @retval -ENOENT If the font index is not available.
 */
int display_set_font(const struct device *dev, uint8_t font_idx);

/**
 * @brief Draw a rectangle to the display buffer
 * @param dev Display device instance
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param w Width
 * @param h Height
 */
void display_draw_rect(const struct device *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/**
 * @brief Flush the display buffer to the hardware (trigger refresh)
 * @param dev Display device instance
 */
void display_flush(const struct device *dev);

#endif /* DISPLAY_LIB_H */
