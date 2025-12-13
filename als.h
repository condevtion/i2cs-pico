#ifndef ALS_H
#define ALS_H

#define ALS_ADDR 0x52

#define ALS_REG_MAIN_CTRL     0x00
#define ALS_REG_LS_MEAS_RATE  0x04
#define ALS_REG_LS_GAIN       0x05
#define ALS_REG_ID            0x06
#define ALS_REG_MAIN_STATUS   0x07
#define ALS_REG_LS_DATA       0x0A
#define ALS_REG_LS_DATA_GREEN 0x0D

#define ALS_MAIN_CTRL_PS_EN    0x01
#define ALS_MAIN_CTRL_LS_EN    0x02
#define ALS_MAIN_CTRL_RGB_MODE 0x04
#define ALS_MAIN_CTRL_SW_RESET 0x10
#define ALS_MAIN_CTRL_SAI_LS   0x20
#define ALS_MAIN_CTRL_SAI_PS   0x40

#define ALS_MAX_CHECKS 3

#define ALS_MEAS_MASK 0x07

#define ALS_MEAS_RATE_25   0 // 40 measurements per second (25 ms interval)
#define ALS_MEAS_RATE_50   1 // 20 measurements per second (50 ms)
#define ALS_MEAS_RATE_100  2 // 10 measurements per second (100 ms) - default
#define ALS_MEAS_RATE_200  3 // 5 measurements per second (200 ms)
#define ALS_MEAS_RATE_500  4 // 2 measurements per second (500 ms)
#define ALS_MEAS_RATE_1000 5 // 1 measurement per second (1000 ms)
#define ALS_MEAS_RATE_2000 6 // 1 measurement per 2 seconds (2000 ms)
#define ALS_MEAS_RATE_2001 7 // 1 measurement per 2 seconds (2000 ms)

#define ALS_MEAS_RES_20 0 // 20-bit resolution - 400 ms integration time
#define ALS_MEAS_RES_19 1 // 19-bit - 200 ms
#define ALS_MEAS_RES_18 2 // 18-bit - 100 ms (default)
#define ALS_MEAS_RES_17 3 // 17-bit - 50 ms
#define ALS_MEAS_RES_16 4 // 16-bit - 25 ms
#define ALS_MEAS_RES_13 5 // 13-bit - 3.125 ms

#define ALS_JIFFY 3125 // Minimum integration time - 3.125 ms

#define ALS_MEAS_GAIN_1  0
#define ALS_MEAS_GAIN_3  1 // default
#define ALS_MEAS_GAIN_6  2
#define ALS_MEAS_GAIN_9  3
#define ALS_MEAS_GAIN_18 4

#define ALS_ID 0xC2

#define ALS_STATUS_PS_DATA  0x01
#define ALS_STATUS_PS_INT   0x02
#define ALS_STATUS_PS_LOGIC 0x04
#define ALS_STATUS_LS_DATA  0x08
#define ALS_STATUS_LS_INT   0x10
#define ALS_STATUS_POWER_ON 0x20

int als_find(uint8_t *addr);
int als_get_id(uint8_t addr, uint8_t *id);

int als_start_measure(uint8_t addr, bool rgb, uint8_t gain, uint8_t res, uint8_t rate, absolute_time_t *deadline);
int als_check_result(uint8_t addr, uint8_t mask);
int als_read_al(uint8_t addr, uint32_t *value);
int als_read_ir_rgb(uint8_t addr, uint32_t *ir, uint32_t *r, uint32_t *g, uint32_t *b);
uint8_t als_get_gain(uint32_t al);
uint8_t als_get_gain_x(uint8_t gain);

#endif //ALS_H
