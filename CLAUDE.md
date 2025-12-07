# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a ZMK firmware configuration repository for a custom keyboard shield named "fat_cruiser". ZMK is a modern, open-source keyboard firmware built on Zephyr RTOS. This repository uses the ZMK user config pattern, where firmware builds are triggered via GitHub Actions.

## Repository Structure

The repository follows ZMK's modular architecture with two main board/shield locations:

- `config/boards/shields/fat_cruiser/` - Primary shield definition (currently active)
- `boards/shields/` - Alternative location for shields (currently empty, contains only .gitkeep)

The shield definition in `config/boards/shields/fat_cruiser/` is the active one and includes:
- `fat_cruiser.keymap` - Keymap configuration in devicetree format
- `fat_cruiser.overlay` - Hardware definition (matrix, pins, peripherals)
- `fat_cruiser.conf` - ZMK feature configuration
- `Kconfig.shield` - Shield detection
- `Kconfig.defconfig` - Default keyboard name configuration

## Build System

The firmware is built automatically via GitHub Actions (defined in `.github/workflows/build.yml`) which uses ZMK's official build workflow (`zmkfirmware/zmk/.github/workflows/build-user-config.yml@v0.3`).

Build configuration is defined in `build.yaml`, which specifies:
- Target board: `nice_nano_v2`
- Shield: `fat_cruiser`

Local builds are not typically performed; the GitHub Actions workflow handles all compilation.

## West Workspace

This repository uses Zephyr's `west` manifest system (`config/west.yml`):
- Points to ZMK v0.3 as the base revision
- Uses `zmkfirmware` GitHub organization as the remote
- The `config/` directory is set as `self.path` in the manifest

The `zephyr/module.yml` file declares this repository as a Zephyr module with `board_root: .`, which allows the build system to discover boards/shields in this repo.

## Hardware Configuration

The fat_cruiser shield is configured for:
- **Board**: nice_nano_v2 (nRF52840-based Pro Micro compatible)
- **Matrix**: 1x1 (single key for testing)
  - Row GPIO: pro_micro pin 8 (active high with pull-down)
  - Column GPIO: pro_micro pin 9 (active high)
  - Diode direction: col2row
- **Display**: SSD1306 OLED (128x64) via I2C
  - Connected to `&pro_micro_i2c` bus
  - I2C address: 0x3c
  - Various display parameters configured (segment-remap, com-invdir, etc.)

## Key Configuration Files

- **build.yaml**: Defines which board/shield combinations to build
- **config/west.yml**: West manifest pointing to ZMK upstream
- **config/boards/shields/fat_cruiser/fat_cruiser.overlay**: Hardware devicetree overlay (pins, matrix, I2C devices)
- **config/boards/shields/fat_cruiser/fat_cruiser.keymap**: Key bindings in devicetree syntax
- **config/boards/shields/fat_cruiser/fat_cruiser.conf**: Kconfig options (display enabled, logging/RGB commented out)

## Making Changes

When modifying the keyboard:

1. **Keymap changes**: Edit `config/boards/shields/fat_cruiser/fat_cruiser.keymap`
   - Uses devicetree syntax with ZMK behaviors (`&kp`, `&mo`, `&lt`, etc.)
   - Layers defined in `keymap` node
   - Include headers: `<behaviors.dtsi>`, `<dt-bindings/zmk/keys.h>`, `<dt-bindings/zmk/bt.h>`

2. **Hardware/matrix changes**: Edit `config/boards/shields/fat_cruiser/fat_cruiser.overlay`
   - Matrix dimensions in `default_transform`
   - GPIO pins in `default_kscan`
   - Peripheral configurations (I2C devices like OLED)

3. **Feature flags**: Edit `config/boards/shields/fat_cruiser/fat_cruiser.conf`
   - Currently has `CONFIG_ZMK_DISPLAY=y` enabled
   - RGB underglow and USB logging are commented out

4. **Build targets**: Edit `build.yaml` to change boards or add build variants

## Recent Hardware Updates

Based on recent commits, the I2C configuration has been updated for Zephyr 4.1.0 compatibility:
- Moved from `pro_micro_i2c` device tree suffix pattern to direct I2C configuration
- Added pinctrl for I2C SDA/SCL pins
- Changed display compatible string to ZMK-specific format (without `-i2c` suffix)

When working with I2C devices, use the current pattern in `fat_cruiser.overlay` which references `&pro_micro_i2c` with proper pinctrl configuration.
