# Repository Guidelines

## Project Structure & Module Organization
- `src/main.c` hosts the Zephyr application entry point and is the only place business logic should live. Keep new peripherals or drawing helpers in separate static functions inside this file until the code grows enough to justify splitting into additional modules under `src/`.
- `boards/nrf52840dk_nrf52840.overlay` contains the SPI1 pin assignments and the `zephyr,mipi-dbi-spi` host that instantiates the SSD16xx node; update this board overlay instead of editing upstream DTS files.
- `prj.conf` holds feature toggles (SPI, DISPLAY, CFB) and stack sizing. Group related options and document any non-default setting inline.
- `build/` is ephemeral output from CMake/Ninja. Wipe it (`rm -rf build`) before switching boards to avoid stale artifacts.

## Build, Test, and Development Commands
- `west build -b nrf52840dk_nrf52840 .` compiles the app for the Nordic DK. The board overlay is picked up automatically when its filename matches the board. Clone+rename the overlay (and adjust `zephyr,display`) when targeting a different board.
- Use `USER_CACHE_DIR=$PWD/.zephyr-cache west build ...` if your environment prevents Zephyr from writing to `/opt/.../.cache`.
- `west flash` flashes the last successful build to the connected target; ensure the J-Link interface is powered.
- `west build -b <board> . -t run` starts the Zephyr emulation backend when available, which is useful for smoke tests that do not touch the E-Paper controller.
- `west build -b <board> . -t menuconfig` launches Kconfig so you can tweak display or SPI settings interactively.

## Coding Style & Naming Conventions
- Follow Zephyr's kernel style: four-space indents, braces on the same line as control statements, and descriptive snake_case identifiers (`epd_init_framebuffer`).
- Keep logging via `printk` concise and prefix hardware-specific messages with the peripheral name (`EPD:`) for easier RTT filtering.
- Run `west clang-format src/main.c` (requires the Zephyr SDK toolchain) before sending changes; add new headers to `CMakeLists.txt` if you introduce additional source files.

## Testing Guidelines
- Prefer Zephyr's `ztest` framework for unit tests; place suites under `tests/` with names matching the module under test (e.g., `tests/cfb_draw/`).
- Hardware validation still matters: capture at least one photo or logic-analyzer trace when adding new display flows so reviewers can confirm refresh timing.
- When tests are unavailable for a feature, describe the manual verification scenario (board, panel revision, commands run) in the PR description.

## Commit & Pull Request Guidelines
- Keep commit subjects imperative and ≤72 characters (`Add SSD168x busy-pin guard`). Squash trivial fixups locally to maintain a linear history similar to the existing `Initial commit for epaper-hello-world`.
- Every PR should link the Jira/GitHub issue it solves, explain the motivation, list verification steps, and include any hardware photos or UART logs relevant to the change.
- Request at least one reviewer familiar with Zephyr display drivers and wait for CI/bench tests to finish before merging.

## Hardware & Configuration Tips
- Ensure the pin definitions in `boards/nrf52840dk_nrf52840.overlay` match your wiring harness (SCK=P0.25, MOSI=P0.23, CS=P0.22, DC=P0.20, RST=P0.19, BUSY=P0.17). Update both the overlay and documentation whenever the mapping changes.
- The overlay's root node sets `zephyr,display = &epd`; keep that label stable so `DEVICE_DT_GET(DT_NODELABEL(epd))` continues to resolve in `src/main.c`.
- Use `CONFIG_MAIN_STACK_SIZE` in `prj.conf` to reserve enough memory for SPI refresh bursts; 2048 bytes suits the Waveshare v4 panel, but high-refresh sequences or additional tasks may require more headroom.
- Keep `CONFIG_HEAP_MEM_POOL_SIZE` around 8 KB so CFB can `k_malloc` the ~3.8 KB framebuffer plus bookkeeping; too small and init fails with `-ENOMEM`.
