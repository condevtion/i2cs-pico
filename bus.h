#ifndef BUS_H
#define BUS_H

#ifndef BUS_FREQ
#define BUS_FREQ    (100 * 1000) // 100 kHz
#endif

#ifndef BUS_TIMEOUT
#define BUS_TIMEOUT (100 * 1000) // 100 ms
#endif

#define BUS_MCU_PULLUP

#ifndef BUS_DEVICE
#define BUS_DEVICE i2c_default
#endif

#ifndef BUS_SDA_PIN
#define BUS_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN
#endif

#ifndef BUS_SCL_PIN
#define BUS_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN
#endif

void bus_setup(void);

int bus_addr_check(uint8_t addr);
const char *bus_addr_check_to_str(int result);

int bus_write_byte(uint8_t addr, uint8_t reg, uint8_t data);

int bus_read_byte(uint8_t addr, uint8_t reg, uint8_t *dst);
int bus_read_word(uint8_t addr, uint8_t reg, uint16_t *dst);
int bus_read_qword(uint8_t addr, uint8_t reg, uint64_t *dst);

#endif //BUS_H
