# SSD1306 OLED Display Debugging Guide

## Problem Summary

SSD1306 OLED display (128x64, from splitkb.com) not working with nice_nano v2 running ZMK firmware. Display shows no output despite correct wiring and various configuration attempts.

## Hardware Setup

### Components
- **Controller**: nice_nano v2 (nRF52840-based)
- **Display**: splitkb.com SSD1306 OLED 128x64
  - Rated voltage: 5V
  - I2C interface
  - Has pull-up resistors R11 and R12 on SDA/SCL lines
- **Keyboard**: Custom "fat_cruiser" shield

### Wiring
- **SDA**: D2 (P0.17) on nice_nano
- **SCL**: D3 (P0.20) on nice_nano
- **GND**: GND
- **VCC**: Tested both VCC (3.3V) and BATTERY+/RAW (5V)

## ZMK Configuration Files

### Key Files Modified
1. `config/boards/shields/fat_cruiser/fat_cruiser.overlay` - Display devicetree configuration
2. `config/boards/shields/fat_cruiser/Kconfig.defconfig` - I2C and driver enablement
3. `config/boards/shields/fat_cruiser/fat_cruiser.conf` - Display feature flag
4. `build.yaml` - USB logging snippet

## Debugging Timeline

### Initial Configuration Issues

**Problem**: Missing I2C driver configuration
- ZMK requires explicit I2C and SSD1306 driver enablement in `Kconfig.defconfig`
- Without this, I2C subsystem doesn't initialize properly

**Solution**: Added I2C and SSD1306 config blocks to `Kconfig.defconfig` within `if ZMK_DISPLAY` conditional

**Reference**: Other working ZMK shields use this pattern

### I2C Address Attempts

Tried multiple I2C addresses:

| Address | Voltage | Error |
|---------|---------|-------|
| 0x3c | 3.3V | `sed: position 0 keycode 0x70004` |
| 0x3d | 3.3V | `b_get_conn_state: state : 1` |
| 0x3d | 5V | `state: 3` (NACK) |
| 0x3c | 5V | `b_get_conn_state: state : 1` |
| 0x3c | 3.3V | `Error 0x0BAE0001` (ANACK) |

**Finding**: Different errors at different addresses/voltages suggested display was being detected but not responding correctly.

### Display Property Configurations Tried

**Attempt 1**: Configuration with `solomon,ssd1306fb-i2c` compatible string (Zephyr 4.1.0 format)
- **Result**: Build failed - property resolution issues

**Attempt 2**: Reverted to `solomon,ssd1306fb` with 100ms ready-time delay
- **Result**: Built successfully but I2C errors persisted

**Attempt 3**: Added `com-sequential` property
- **Result**: No improvement

**Attempt 4**: Matched minimal configuration used by other working boards
- **Result**: Still ANACK errors

**Attempt 5**: Slowed I2C clock to 100kHz
- **Result**: No improvement

### USB Logging Configuration

**Initial attempt**: `CONFIG_ZMK_USB_LOGGING=y` in `.conf` file
- **Result**: Build failed with Kconfig warnings on Zephyr 4.1.0

**Solution**: Use ZMK snippet `zmk-usb-logging` in `build.yaml`
- **Result**: Successful builds with USB logging enabled

## Error Codes Explained

### 0x0BAE0001 (NRFX_ERROR_DRV_TWI_ERR_ANACK)
- **Meaning**: Address Not Acknowledged
- **Cause**: I2C slave device not responding to the address byte
- **Possible reasons**:
  - Wrong I2C address
  - Device not powered correctly
  - Missing/weak pull-up resistors
  - Defective module
  - Wrong voltage

**Source**: [Nordic nrfx errors.h](https://github.com/NordicSemiconductor/nrfx/blob/master/drivers/nrfx_errors.h)

### State: 3 (NRF_ERROR_INTERNAL)
- **Meaning**: Hardware detected NACK
- **Cause**: Slave sends NACK during communication
- **Different from ANACK**: Can occur on data bytes, not just address

**Source**: [Nordic DevZone - Error code 3](https://devzone.nordicsemi.com/f/nordic-q-a/69238/error-code-3-passed-to-twi-manager-callback)

### Other errors encountered
- `sed: position 0 keycode 0x70004` - Early I2C line error
- `b_get_conn_state: state : 1` - Likely Bluetooth state logging coinciding with I2C error

## Key Findings

### 1. Voltage Incompatibility
- **splitkb OLED is rated for 5V**
- nice_nano v2 provides only **3.3V on VCC pin**
- BATTERY+/RAW provides ~5V when USB connected, but ~3.7-4.2V on battery
- **ANACK errors occurred at both 3.3V and 5V**, suggesting voltage alone wasn't the only issue

### 2. nice_nano I2C Pin Mappings
Confirmed from ZMK source code:
- **D2 → P0.17 (SDA)**
- **D3 → P0.20 (SCL)**

**NOT** D0/D1 as initially thought.

**Source**: [ZMK nice_nano pins](https://github.com/zmkfirmware/zmk/blob/main/app/boards/arm/nice_nano/arduino_pro_micro_pins.dtsi)

### 3. Required Kconfig Configuration
ZMK shields **must** explicitly enable I2C and display drivers in `Kconfig.defconfig` within an `if ZMK_DISPLAY` conditional block.

This was missing from the initial configuration and is **critical** for I2C initialization.

**Pattern used by working shields**: I2C and SSD1306 drivers are enabled with `default y` within the display conditional.

### 4. Display Properties
Other working boards use minimal properties:
- No `segment-remap` or `com-invdir`
- No `ready-time-ms` delay
- Simple configuration with just basic dimensions and precharge period

### 5. Pull-up Resistors
- Display module has built-in pull-ups (R11, R12)
- nRF52840 TWI requires external pull-ups for reliable operation
- Module's pull-ups appear sufficient (no missing pull-up errors)

### 6. Reset Pin
- Reset pin is **optional** for I2C SSD1306 displays
- Not required in devicetree configuration
- Display will initialize without it via I2C commands

**Source**: [Zephyr I2C Display Discussion](https://github.com/zephyrproject-rtos/zephyr/discussions/69310)

## Final Configuration Summary

### Kconfig.defconfig
- Added `if ZMK_DISPLAY` block
- Enabled `config I2C` with `default y`
- Enabled `config SSD1306` with `default y`

### fat_cruiser.overlay
- I2C bus: `&pro_micro_i2c` with `status = "okay"`
- Clock frequency: `100000` (100kHz for compatibility)
- Display address: `0x3c`
- Compatible: `"solomon,ssd1306fb"`
- Dimensions: 128x64
- Minimal properties: width, height, offsets, multiplex-ratio, prechargep, inversion-on

### fat_cruiser.conf
- `CONFIG_ZMK_DISPLAY=y`

### build.yaml
- Added `snippet: zmk-usb-logging` for diagnostic logging

## Conclusion

Despite proper configuration matching patterns from other working ZMK shields, the splitkb SSD1306 display did not work with nice_nano v2. **Root cause identified**: Voltage incompatibility.

### Why It Failed
1. **Primary issue**: Display rated for 5V, nice_nano provides 3.3V
2. **Secondary issues**:
   - BATTERY+/RAW only provides 5V when USB connected (not on battery)
   - Even with 5V from RAW, ANACK errors persisted
   - Possible hardware incompatibility or defective module
   - Some SSD1306 modules have solder jumpers that change I2C address

### Solution
Ordered a **3.3V-compatible SSD1306** display, which should work properly with nice_nano v2's native 3.3V output.

## Lessons Learned

### For Future ZMK Shield Development

1. **Always check voltage compatibility** - nice_nano v2 is 3.3V, many OLED modules are 5V
2. **Enable I2C in Kconfig.defconfig** - Not automatic, must be explicit
3. **Start with minimal configuration** - Match patterns from other working boards
4. **Use proper I2C pins** - D2/D3 on nice_nano, not D0/D1
5. **Enable USB logging via snippet** - Not via CONFIG in .conf file
6. **Check for hardware variations** - Some displays have solder jumpers for address selection
7. **Verify pull-up resistors** - Check module has resistors near I2C pins

### Debugging I2C Issues

1. **Check pin mappings** in board's devicetree files
2. **Verify pull-up resistors** are present (either on module or via internal pull-ups)
3. **Test both common addresses** (0x3c and 0x3d)
4. **Try slower clock speeds** (100kHz for compatibility)
5. **Match working configurations** from similar keyboards
6. **Monitor error codes** to distinguish between different failure modes:
   - `0x0BAE0001` = Address NACK (wrong address or no device)
   - `state: 3` = Data NACK (device responding but rejecting commands)
7. **Test voltage at display VCC pin** with multimeter
8. **Check for solder jumpers** on display module back

### Common Mistakes to Avoid

1. **Wrong pin numbers** - Verify D2/D3 not D0/D1 for I2C on nice_nano
2. **Missing Kconfig.defconfig** - I2C won't initialize without explicit enable
3. **Wrong USB logging method** - Use snippet, not CONFIG option
4. **Assuming voltage compatibility** - Check display datasheet
5. **Over-configuring display** - Start minimal, add properties only if needed
6. **Wrong compatible string** - Use `solomon,ssd1306fb` not `solomon,ssd1306fb-i2c` for ZMK v0.3

## Useful References

### Documentation
- [ZMK Display Config](https://zmk.dev/docs/config/displays)
- [Zephyr SSD1306 I2C Binding](https://docs.zephyrproject.org/latest/build/dts/api/bindings/display/solomon,ssd1306fb-i2c.html)
- [ZMK USB Logging](https://zmk.dev/docs/development/usb-logging)
- [ZMK New Shield Guide](https://zmk.dev/docs/development/hardware-integration/new-shield)

### ZMK Source Reference
- [Official ZMK shields folder](https://github.com/zmkfirmware/zmk/tree/main/app/boards/shields) - Examples with SSD1306
- [nice_nano v2 pinout](https://github.com/zmkfirmware/zmk/blob/main/app/boards/arm/nice_nano/arduino_pro_micro_pins.dtsi)

### Troubleshooting Resources
- [Nordic DevZone - I2C/TWI](https://devzone.nordicsemi.com/f/nordic-q-a) - nRF52840 I2C issues
- [ZMK GitHub Issues - Display tag](https://github.com/zmkfirmware/zmk/issues?q=is%3Aissue+label%3Adisplay)
- [ZMK Discord](https://zmk.dev/community/discord/invite) - Community support

### Hardware
- [nice_nano documentation](https://nicekeyboards.com/docs/nice-nano/)
- [nice_nano pinout](https://nicekeyboards.com/docs/nice-nano/pinout-schematic/)
- [splitkb OLED product page](https://splitkb.com/products/oled-display)
- [Nordic nRF52840 TWI documentation](https://infocenter.nordicsemi.com/topic/ps_nrf52840/twi.html)

## Build History

Final successful builds:
- **Run #19994204182** - I2C and SSD1306 driver configuration added
- **Run #19994369052** - I2C clock slowed to 100kHz

All builds: https://github.com/mackeper/zmk-config-fat_cruiser/actions

## Next Steps

When 3.3V-compatible SSD1306 arrives:
1. Verify it's rated for 3.3V operation
2. Connect to VCC (3.3V) pin, not BATTERY+/RAW
3. Use existing configuration (should work with minimal changes)
4. If issues persist, try address 0x3d
5. Check for solder jumpers on new module
