cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(epaper-hello-world)

target_sources(app PRIVATE
	src/main.c
	src/epd_driver.c
	src/epd_graphics.c
	src/display_lib.c
	src/cfb_font_digits_3056.c
)
