Snowflake, a New Year Souvenir
------------------------------

These were small New Year souvenirs I've made for my friends and family.

Snowflake shaped piece of laser cut acrylic with a small PCB in the center. PCB has 6 right angle mounted LEDs illuminating the inside of the acrylic sheet. LEDs are driven by the microcontroller, which uses PWM to make LEDs glow in a pseudo random way. One button, connected to the reset pin of the microcontroller, is used to turn on and turn off the device.

### Laser cut shapes

`lasercut` directory contains a Sketchup file with two snowflake shapes.

![laser cut shapes](https://github.com/igrr/snowflake/raw/master/lasercut/snowflakes.png "Two snoflake shapes for laser cutting")

### PCB

`pcb` directory contains the schematic and PCB layout made in Eagle CAD.

![schematic](https://github.com/igrr/snowflake/raw/master/pcb/snowflake_insert_sch.png "Schematic")

### Firmware

`firmware` directory contains the program for the STM8L051F3 MCU. To build it:
- install [SDCC](https://sourceforge.net/projects/sdcc) and add it to the `PATH`
- run `git submodule update --init` to get libstm8 submodule
- go to `firmware` directory and run `make`

[stm8flash](https://github.com/vdudouyt/stm8flash) wan used to flash the firmware to the MCU. Compile it, and add it to the `PATH`. Then run `make flash` in `firmware` directory.

### Parts used

- LEDs: https://item.taobao.com/item.htm?id=525144507380
- 0603 1k resistors
- CR1220 battery: https://item.taobao.com/item.htm?id=521527440135
- STM8L051F3P6 MCU: https://detail.tmall.com/item.htm?id=40540345444

### License

All files in this repository (except libstm8 submodule) are distributed under CC0 license.
