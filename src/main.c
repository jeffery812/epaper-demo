#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(epaper_app, LOG_LEVEL_DBG);

static void dump_display_info(const struct device *dev)
{
	int width = cfb_get_display_parameter(dev, CFB_DISPLAY_WIDTH);
	int height = cfb_get_display_parameter(dev, CFB_DISPLAY_HEIGHT);
	int ppt = cfb_get_display_parameter(dev, CFB_DISPLAY_PPT);
	int rows = cfb_get_display_parameter(dev, CFB_DISPLAY_ROWS);
	int cols = cfb_get_display_parameter(dev, CFB_DISPLAY_COLS);
	int fonts = cfb_get_numof_fonts(dev);

	LOG_INF("CFB metrics -> width:%d height:%d ppt:%d rows:%d cols:%d fonts:%d",
		width, height, ppt, rows, cols, fonts);
}

static int print_line(const struct device *dev, const char *msg, uint16_t x, uint16_t y)
{
	int err = cfb_print(dev, msg, x, y);

	if (err) {
		LOG_ERR("cfb_print(\"%s\") failed at (%u,%u) (%d)", msg, x, y, err);
	}

	return err;
}

int main(void)
{
	const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(epd));
	int err;

	LOG_INF("epaper-hello-world boot");

	if (!device_is_ready(dev)) {
		LOG_ERR("E-Paper device %s not ready", dev->name);
		return -EIO;
	}

	LOG_INF("Using display device %s", dev->name);

	if (display_set_pixel_format(dev, PIXEL_FORMAT_MONO10) &&
	    display_set_pixel_format(dev, PIXEL_FORMAT_MONO01)) {
		LOG_ERR("Failed to set MONO pixel format");
		return -ENODEV;
	}

	err = cfb_framebuffer_init(dev);
	if (err) {
		LOG_ERR("Framebuffer init failed (%d)", err);
		return err;
	}

	dump_display_info(dev);

	err = cfb_framebuffer_set_font(dev, 0);
	if (err) {
		LOG_WRN("Unable to set font 0 (%d), continuing with default", err);
	}

	err = cfb_framebuffer_clear(dev, true);
	if (err) {
		LOG_ERR("Framebuffer clear failed (%d)", err);
		return err;
	}

	err = display_blanking_off(dev);
	if (err && err != -ENOTSUP) {
		LOG_WRN("display_blanking_off failed (%d)", err);
	}

	if (print_line(dev, "Hello ePark!", 10, 10) ||
	    print_line(dev, "Waveshare E-Paper", 10, 30) ||
	    print_line(dev, "v4.0 Ready", 10, 50)) {
		LOG_ERR("Printing failed, aborting refresh");
		return -EIO;
	}

        LOG_INF("begin physical refresh...");
	err = cfb_framebuffer_finalize(dev);
        LOG_INF("physical refresh done. waiting another 5 seconds...");
        k_sleep(K_SECONDS(5)); // 强制等5秒，观察屏幕有没有“闪烁”一下
	if (err) {
		LOG_ERR("Framebuffer finalize failed (%d)", err);
		return err;
	}

	LOG_INF("E-Paper refresh complete");

	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
