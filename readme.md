The firmware for *milandr 1986ve1t mcu*, that implements a minimal UDP/IP stack, with DHCP support, allowing to work with LCD, LEDs and buttons via network, and is dependency-free.

## Build
### Requirements
* python
* meson
* ninja
* arm-embedded-gcc (arm-none-eabi-gcc)

### Commands
```
meson setup buildDir/ --cross extra/1986ve1t.cross
meson compile -C buildDir/ firmware.elf
```
The firmware file will be located in `buildDir/firmware/firmware.elf`
