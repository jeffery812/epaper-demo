#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(raw_epd, LOG_LEVEL_INF);

/* SPI configuration */
#define SPI_OP  (SPI_OP_MODE_MASTER | SPI_WORD_SET(8))

static const struct spi_dt_spec spi_dev = SPI_DT_SPEC_GET(DT_NODELABEL(raw_spi), SPI_OP, 0);

static const struct gpio_dt_spec busy_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(busy_pin), gpios);
static const struct gpio_dt_spec rst_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(reset_pin), gpios);
static const struct gpio_dt_spec dc_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(dc_pin), gpios);

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
    epd_send_data(0x03);

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
    epd_send_data(0xF9); // 249
    epd_send_data(0x00);
    epd_send_data(0x00);
    epd_send_data(0x00);
}

void epd_clear_screen(void)
{
    uint16_t width_bytes = 16; 
    uint16_t height = 250;
    
    epd_send_cmd(0x24); // Write RAM
    for (int i = 0; i < width_bytes * height; i++) {
        epd_send_data(0xFF); // White
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
    
    LOG_INF("Clearing Screen (White)...");
    epd_clear_screen();

    LOG_INF("Test Complete.");
    return 0;
}
