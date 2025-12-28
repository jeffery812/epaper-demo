# E-Paper Hello World (nRF52840 + Waveshare 2.13" V4)

This is a Zephyr RTOS application demonstrating how to drive a **Waveshare 2.13-inch e-Paper V4** display using an **nRF52840 DK**.

It features a custom driver implementation to handle the specific "Soft Start" initialization sequence required by the V4 hardware revision, which is not currently supported by the standard Zephyr `ssd16xx` driver. It also implements a display driver wrapper to utilize Zephyr's **Character Framebuffer (CFB)** subsystem for easy text and graphics rendering.

## Hardware Requirements

*   **Development Board**: Nordic nRF52840 DK (PCA10056)
*   **Display**: Waveshare 2.13inch e-Paper Module (Version 4)
*   **Connection**: SPI

## Wiring

Connect the E-Paper display to the nRF52840 DK as follows (based on the device tree overlay):

| E-Paper Pin | nRF52840 DK Pin | Arduino Header | Description |
| :--- | :--- | :--- | :--- |
| **VCC** | 3.3V | - | Power (3.3V) |
| **GND** | GND | - | Ground |
| **DIN** | P1.13 | D11 | SPI MOSI |
| **CLK** | P1.15 | D13 | SPI SCK |
| **CS** | P1.12 | D10 | SPI Chip Select |
| **DC** | P1.11 | D9 | Data/Command |
| **RST** | P1.10 | D8 | Reset |
| **BUSY** | P1.06 | - | Busy Signal |

> **Note**: The BUSY pin is connected to **P1.06**. Please ensure you are using the correct pin on the P1 header, not P0.06.

## Software Architecture

This project bypasses the standard `ssd16xx` driver to solve compatibility issues with the V4 screen.

1.  **`src/epd_driver.c`**: Low-level driver. Handles SPI communication, GPIO control, and the specific initialization sequence (including the critical "Soft Start" command `0x0C`) required to wake up the V4 screen's charge pump.
2.  **`src/epd_graphics.c`**: A wrapper that implements the Zephyr `display_driver_api`. It acts as a bridge, allowing the high-level Zephyr CFB subsystem to draw into a local buffer. It also handles **90-degree rotation** to display content in Landscape mode.
3.  **`src/main.c`**: Application logic. Uses `cfb_print`, `cfb_draw_rect`, etc., to render content.

## Building and Flashing

This project uses the nRF Connect SDK (Zephyr).

### 1. Build

```bash
west build -b nrf52840dk_nrf52840
```

### 2. Flash

```bash
west flash
```

## Key Features

*   **Waveshare V4 Support**: Includes the specific "Soft Start" parameters (`0xAE, 0xC7, 0xC3, 0xC0, 0x80`) required to drive the V4 panel.
*   **Landscape Mode**: The driver wrapper automatically rotates the CFB buffer 90 degrees so text appears horizontally.
*   **Zephyr CFB Integration**: Uses standard Zephyr APIs for drawing text and shapes, making it easy to extend.
*   **Robust SPI**: Configured for 1MHz SPI to ensure signal integrity over jumper wires.

## Troubleshooting

*   **Screen not refreshing**:
    *   Check wiring, especially the **BUSY** pin (P1.06).
    *   Ensure the FPC cable is fully inserted into the connector on the HAT.
    *   Verify the "Soft Start" patch is active in `epd_driver.c`.
*   **Garbage pixels**:
    *   This usually indicates an MSB/LSB mismatch. The driver is configured for `SCREEN_INFO_MONO_MSB_FIRST`.

## License

Apache 2.0
```
