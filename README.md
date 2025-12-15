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

## Hardware

Connect I2C Sensors board to your Raspberry Pi Pico 2 as shown below:

<img src="https://github.com/user-attachments/assets/7fc2740f-b765-499b-9fae-7559cd859e09" alt="I2C Sensors Breadboard Sketch" width="512" />

The sensor board takes power form Pico 2 3.3V rail (pin 36), uses I2C0 interface: SDA - pin 6, SCL - 7, and GPIO22 (pin 29) to listen for pressure sensor interrupt. Don't forget to connect ground pins (for example, use closest one from Pico - pin 18). As the I2CS board has physicall pullup resistors on I2C lines there is no need to pull them up from MCU's side but it may allow detect absence of proper connection faster. On the other hand GPIO22 needs to be pulled up as default active state for pressure sensor interrupt is low and the sensor doesn't keep it high otherwise.

With the binary from build step uploaded Pico should enumerate sensors, start measurements, and report details and results to its USB-serial interface.

<details>

<summary>Click to see an example of the output</summary>

```
.........
I2CS Board RPi2 Example

Enumerating sensors:
        SPL07-003: 0x76
        ENS210...: 0x43
        APDS-9999: 0x52

Checking sensor IDs:
        SPL07-003 ID: 1.1
        ENS210 ID...: 210.2 (6020514E24E96E0A)
        APDS-9999 ID: 12.2

SPL07-003 Calibration Coefficients:
        c0:     +272, c00:   +81549, c01:    -1753
        c1:     -278, c10:   -60333, c11:     +741
                      c20:   -10834, c21:     -104
                      c30:    -1759, c31:      -82
                      c40:     -302

Configuring pressure sensor (p scale 253952, t scale 524288)... ok
Starting at offset 2 (1989 ms remains for the first cycle)
1: 4.011 s
Interrupt pin(22): 1
Starting temperature measurement with pressure sensor... ok
SPL07-003 - done (int@22 - 0, 4.015 s): 3654 us (4 clk)
Reading pressure sensor operating status... PRS_RDY: N, TMP_RDY: Y
Interrupt status: INT_PRS: N, INT_TMP: Y, INT_FIFO: N
Interrupt pin(22): 1
Reading raw temperature value from pressure sensor... t_raw: 214190, t_raw_sc: 0.4085350
Interrupt pin(22): 1
Starting pressure measurement... ok
Starting relative humidity measurement... ok
Starting ambient light probing... ok
APDS-9999 - done(4.022 s): 3551 us
Reading preliminary ambient light sensor data... ok
APDS-9999 (probe): AL: 154.0 lux (0x000014), gain: 18x
Starting ambient light measurement... ok
SPL07-003 - done (int@22 - 0, 4.045 s): 27275 us (18 clk)
Reading pressure sensor operating status... PRS_RDY: Y, TMP_RDY: N
Interrupt status: INT_PRS: Y, INT_TMP: N, INT_FIFO: N
Interrupt pin(22): 1
Reading raw pressure value... p_raw: -89601, p_raw_sc: -0.3528265
SPL07-003 - T: +22.4 C, P: 1007.33 mbar
ENS210 - done(4.148 s): 130000 us
Reading relative humidity sensor data... ok
ENS210 T.: +22.23 C (0x49d8), valid: Y, CRC: 0x1a (ok)
ENS210 RH:  52.2 %  (0x6869), valid: Y, CRC: 0x00 (ok)
APDS-9999 - done(4.425 s): 400421 us
Reading ambient light data... ok
APDS-9999 AL: 231.5 lux (0x033073 @ 0.007)
Starting color measurement... ok
APDS-9999 - done(4.827 s): 400419 us
Reading color data... ok
APDS-9999 AL - IR: 0x005943, R: 0x020285, G: 0x033092, B: 0x015412
```

</details>

## Code

The code split into several files following its abstraction levels:
* **main.c** - contains overall workflow, including main measurement loop
* **check.c/.h** - pair contains code to enumerate and identify connected sensors
* **meas.c/.h** - contain high-level functions for starting measurements and getting results for each sensor
* **prs.c/.h** - define constants and basic communication functions for SPL07-003 pressure sensor
* **rhs.c/.h** - the same for ENS210 relative humidity sensor
* **als.c/.h** - the same for APDS-9999 ambient light sensor
* **bus.c/.h** - simplifies I2C I/O

General workflow consists of several steps:
1. 4 seconds delay to make catching USB-serial output more convenient
2. Interrupt and I2C bus setup
3. Enumerating and identifying connected sensors
4. Pressure sensor setup
5. Measurement timing alignment
6. Endless measurement loop

Measurements are done mostly in parallel with manual scheduling of readiness checks and data reads:
* The cycle begins by starting the temperature measurement via the pressure sensor.
* Parallel Execution: Immediately following the temperature trigger, the code starts three other processes simultaneously:
  * Pressure measurement
  * Relative Humidity measurement
  * Light probing (to determine proper gain for APDS-9999 sensor)
* Light Probing (~3-4 ms): As the fastest operation, the APDS-9999 sensor is polled first. Based on this result, the gain is calculated, and a precision light measurement is started immediately.
* Pressure Read (~27 ms): The result from the pressure sensor is retrieved.
* Humidity Read (~130 ms): The relative humidity sensor is polled and read.
* Ambient Light & IR/RGB (each ~400 ms): The ambient light result is retrieved. Once this finishes, a final measurement is triggered to capture IR and RGB levels.
* Time alignment: The MCU enters sleep mode to align the start of the next loop with the next whole number of second.

Here is illustration of the sequence (not to scale):

<img src="https://github.com/user-attachments/assets/d889ae4d-bfde-4338-b1be-3274cd9d43c8" alt="I2C Pico Timings" width="800" />

With any changes to sensor settings the timings may change as well and calls to `read_..._data` functions may need to be swapped accordingly. For example, with pressure oversample rate 128 (`PRS_PRC_128`) pressure measurements takes ~207 ms and `read_prs_raw_data` with `calc_prs` should be called after `read_rhs_data`.
