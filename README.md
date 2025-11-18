# i2cs-pico

## About

The i2cs-pico is an example of integration I2C Sensors Board with Raspberry Pi Pico 2.

## Prerequisites

The project requires Pico SDK, Arm GNU Toolchain, make, and CMake. Build tested with SDK 2.2.0 Release - [github.com/raspberrypi/pico-sdk/a1438dff](https://github.com/raspberrypi/pico-sdk/commit/a1438dff1d38bd9c65dbd693f0e5db4b9ae91779).

## Build

To build the example create a build directroy, generate build files, and run make.

```
mkdir build
cd build
cmake -DPICO_BOARD=pico2 ..
make
```

This should produce i2cs-pico.uf2 which then can be copied to RPi Pico in USB Mass Storage mode.
