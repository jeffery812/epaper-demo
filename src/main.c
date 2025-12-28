#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "epd_driver.h"
#include "epd_graphics.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    LOG_INF("Raw SPI E-Paper Test Start");

    if (epd_hardware_init() != 0) {
        LOG_ERR("Hardware init failed");
        return 0;
    }

    LOG_INF("Hardware Initialized. Starting Sequence...");
    
    epd_init_v4();
    
    /* Clear buffer to white (0xFF) */
    epd_clear_buffer(0xFF);

    /* Draw text */
    LOG_INF("Drawing 'hello epaper'...");
    draw_string(10, 50, "hello", 2);
    draw_string(10, 70, "epaper", 2);

    /* Send to display */
    epd_display_framebuffer(framebuffer, sizeof(framebuffer));

    LOG_INF("Test Complete.");
    return 0;
}
