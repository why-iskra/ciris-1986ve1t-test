This firmware implements a minimal udp/IP stack, with DHCP support, allowing you to work with LCD, LEDs and buttons via network.

# Build
## Requirements
* python
* meson
* ninja
* arm-embedded-gcc (arm-none-eabi-gcc)

## Commands
```
meson setup buildDir/ --cross extra/1986ve1t.cross
meson compile -C buildDir/ firmware.elf
```
The firmware file will be located in `buildDir/firmware/firmware.elf`
