#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(raw_epd, LOG_LEVEL_INF);

/* SPI configuration */
#define SPI_OP  (SPI_OP_MODE_MASTER | SPI_WORD_SET(8))

static const struct spi_dt_spec spi_dev = SPI_DT_SPEC_GET(DT_NODELABEL(raw_spi), SPI_OP, 0);

static const struct gpio_dt_spec busy_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(busy_pin), gpios);
static const struct gpio_dt_spec rst_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(reset_pin), gpios);
static const struct gpio_dt_spec dc_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(dc_pin), gpios);

/* Framebuffer: 128 pixels (16 bytes) wide * 250 pixels high */
#define EPD_WIDTH_BYTES 16
#define EPD_HEIGHT      250
static uint8_t framebuffer[EPD_WIDTH_BYTES * EPD_HEIGHT];

static void epd_reset(void)
{
    if (!gpio_is_ready_dt(&rst_gpio)) return;
    
    gpio_pin_set_dt(&rst_gpio, 1); // Active (Low)
    k_msleep(20);
    gpio_pin_set_dt(&rst_gpio, 0); // Inactive (High)
    k_msleep(20);
}

static void epd_send_cmd(uint8_t cmd)
{
    /* DC Low = Command */
    gpio_pin_set_dt(&dc_gpio, 1); 
    struct spi_buf buf = {.buf = &cmd, .len = 1};
    struct spi_buf_set buf_set = {.buffers = &buf, .count = 1};
    spi_write_dt(&spi_dev, &buf_set);
}

static void epd_send_data(uint8_t data)
{
    /* DC High = Data */
    gpio_pin_set_dt(&dc_gpio, 0);
    struct spi_buf buf = {.buf = &data, .len = 1};
    struct spi_buf_set buf_set = {.buffers = &buf, .count = 1};
    spi_write_dt(&spi_dev, &buf_set);
}

static void epd_wait_busy(void)
{
    LOG_INF("Waiting for BUSY...");
    int timeout = 500; // 5 seconds
    while (gpio_pin_get_dt(&busy_gpio) == 1) {
        k_msleep(10);
        if (--timeout == 0) {
            LOG_ERR("BUSY Timeout!");
            break;
        }
    }
    LOG_INF("BUSY Released.");
}

void epd_init_v4(void)
{
    epd_reset();
    epd_wait_busy();

    LOG_INF("Sending Init Commands...");
    
    epd_send_cmd(0x12); // SW Reset
    epd_wait_busy();

    epd_send_cmd(0x01); // Driver output control
    epd_send_data(0xF9);
    epd_send_data(0x00);
    epd_send_data(0x00);

    epd_send_cmd(0x11); // Data entry mode
    epd_send_data(0x03); // X increment, Y increment

    epd_send_cmd(0x3C); // BorderWavefrom
    epd_send_data(0x05);

    epd_send_cmd(0x18); // Temp Sensor
    epd_send_data(0x80); // Internal

    /* Soft Start Patch for V4 */
    LOG_INF("Sending Soft Start Patch...");
    epd_send_cmd(0x0C); 
    epd_send_data(0xAE);
    epd_send_data(0xC7);
    epd_send_data(0xC3);
    epd_send_data(0xC0);
    epd_send_data(0x80); 

    epd_send_cmd(0x44); // Set Ram-X
    epd_send_data(0x00);
    epd_send_data(0x0F); // 128/8 - 1 = 15

    epd_send_cmd(0x45); // Set Ram-Y
    epd_send_data(0x00); // Start 0
    epd_send_data(0x00);
    epd_send_data(0xF9); // End 249
    epd_send_data(0x00);
}

/* Simple 8x8 Bitmap Font Helper */
static const uint8_t *get_char_bitmap(char c) {
    /* Minimal font for "hello epaper" */
    static const uint8_t char_h[] = {0x00, 0x40, 0x40, 0x5C, 0x62, 0x62, 0x62, 0x00};
    static const uint8_t char_e[] = {0x00, 0x00, 0x3C, 0x42, 0x7E, 0x40, 0x3C, 0x00};
    static const uint8_t char_l[] = {0x00, 0x60, 0x20, 0x20, 0x20, 0x20, 0x3C, 0x00};
    static const uint8_t char_o[] = {0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x00};
    static const uint8_t char_p[] = {0x00, 0x00, 0x5C, 0x62, 0x62, 0x5C, 0x40, 0x40};
    static const uint8_t char_a[] = {0x00, 0x00, 0x3C, 0x02, 0x3E, 0x42, 0x3E, 0x00};
    static const uint8_t char_r[] = {0x00, 0x00, 0x5C, 0x62, 0x40, 0x40, 0x40, 0x00};
    static const uint8_t char_sp[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    switch(c) {
        case 'h': return char_h;
        case 'e': return char_e;
        case 'l': return char_l;
        case 'o': return char_o;
        case 'p': return char_p;
        case 'a': return char_a;
        case 'r': return char_r;
        default: return char_sp;
    }
}

void draw_pixel(int x, int y, int color) {
    if (x < 0 || x >= (EPD_WIDTH_BYTES * 8) || y < 0 || y >= EPD_HEIGHT) return;
    
    int idx = y * EPD_WIDTH_BYTES + (x / 8);
    uint8_t mask = 0x80 >> (x % 8);

    if (color) {
        framebuffer[idx] |= mask; // White
    } else {
        framebuffer[idx] &= ~mask; // Black
    }
}

void draw_char(int x, int y, char c, int scale) {
    const uint8_t *bitmap = get_char_bitmap(c);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Check bit j in row i */
            if (bitmap[i] & (0x80 >> j)) {
                for (int dy = 0; dy < scale; dy++) {
                    for (int dx = 0; dx < scale; dx++) {
                        draw_pixel(x + j * scale + dx, y + i * scale + dy, 0); // Black
                    }
                }
            }
        }
    }
}

void draw_string(int x, int y, const char *str, int scale) {
    while (*str) {
        draw_char(x, y, *str, scale);
        x += 8 * scale;
        str++;
    }
}

void epd_display_framebuffer(void)
{
    /* Set counters to 0,0 */
    epd_send_cmd(0x4E); 
    epd_send_data(0x00);
    epd_send_cmd(0x4F); 
    epd_send_data(0x00);
    epd_send_data(0x00);
    
    epd_send_cmd(0x24); // Write RAM
    for (int i = 0; i < sizeof(framebuffer); i++) {
        epd_send_data(framebuffer[i]);
    }
    
    LOG_INF("Activating Display...");
    epd_send_cmd(0x22); // Display Update Control 2
    epd_send_data(0xF7); // Load LUT from OTP + Display
    
    epd_send_cmd(0x20); // Master Activation
    epd_wait_busy();
}

int main(void)
{
    LOG_INF("Raw SPI E-Paper Test Start");

    if (!spi_is_ready_dt(&spi_dev)) {
        LOG_ERR("SPI device not ready");
        return 0;
    }
    
    if (!gpio_is_ready_dt(&busy_gpio) || !gpio_is_ready_dt(&rst_gpio) || !gpio_is_ready_dt(&dc_gpio)) {
        LOG_ERR("GPIO devices not ready");
        return 0;
    }

    gpio_pin_configure_dt(&busy_gpio, GPIO_INPUT);
    gpio_pin_configure_dt(&rst_gpio, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&dc_gpio, GPIO_OUTPUT);

    LOG_INF("Hardware Initialized. Starting Sequence...");
    
    epd_init_v4();
    
    /* Clear buffer to white (0xFF) */
    memset(framebuffer, 0xFF, sizeof(framebuffer));

    /* Draw text */
    LOG_INF("Drawing 'hello epaper'...");
    draw_string(10, 50, "hello", 2);
    draw_string(10, 70, "epaper", 2);

    /* Send to display */
    epd_display_framebuffer();

    LOG_INF("Test Complete.");
    return 0;
}
