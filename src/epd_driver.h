/* src/epd_driver.h */
#ifndef EPD_DRIVER_H
#define EPD_DRIVER_H

#include <stdint.h>
#include <stddef.h>

/* Display resolution */
#define EPD_WIDTH       128
#define EPD_HEIGHT      250
#define EPD_WIDTH_BYTES (EPD_WIDTH / 8)

/**
 * @brief Initialize SPI and GPIO hardware
 * @return 0 on success, negative errno on failure
 */
int epd_hardware_init(void);

/**
 * @brief Run the initialization sequence (Waveshare V4 specific)
 */
void epd_init_v4(void);

/**
 * @brief Send framebuffer to display and trigger refresh
 * @param buffer Pointer to the framebuffer data
 * @param size Size of the buffer in bytes
 */
void epd_display_framebuffer(const uint8_t *buffer, size_t size);

#endif /* EPD_DRIVER_H */