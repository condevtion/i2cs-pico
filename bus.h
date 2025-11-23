#ifndef BUS_H
#define BUS_H

void bus_setup(void);

int bus_addr_check(uint8_t addr);
const char *bus_addr_check_to_str(int result);

int bus_write_byte(uint8_t addr, uint8_t reg, uint8_t data);

int bus_read_byte(uint8_t addr, uint8_t reg, uint8_t *dst);
int bus_read_word(uint8_t addr, uint8_t reg, uint16_t *dst);
int bus_read_qword(uint8_t addr, uint8_t reg, uint64_t *dst);

#endif //BUS_H
