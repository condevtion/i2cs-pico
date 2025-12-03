#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/i2c.h"

#include "bus.h"

#define BUS_SIZE_BYTE  1
#define BUS_SIZE_WORD  2
#define BUS_SIZE_QWORD 8

const char *bus_error_to_str(int result)
{
	switch (result)
	{
	case PICO_OK:
		return "ok";
	case PICO_ERROR_GENERIC:
		return "error";
	case PICO_ERROR_TIMEOUT:
		return "timeout";
	}

	return "unknown";
}

void bus_setup(void)
{
	bi_decl(bi_2pins_with_func(BUS_SDA_PIN, BUS_SCL_PIN, GPIO_FUNC_I2C));

	i2c_init(BUS_DEVICE, BUS_FREQ);
	gpio_set_function(BUS_SDA_PIN, GPIO_FUNC_I2C);
	gpio_set_function(BUS_SCL_PIN, GPIO_FUNC_I2C);

#ifdef BUS_MCU_PULLUP
	gpio_pull_up(BUS_SDA_PIN);
	gpio_pull_up(BUS_SCL_PIN);
#endif
}

int bus_addr_check(uint8_t addr)
{
	uint8_t data;

	int r = i2c_read_timeout_us(BUS_DEVICE, addr, &data, BUS_SIZE_BYTE, false, BUS_TIMEOUT);
	if (r < 0) return r;
	if (r != BUS_SIZE_BYTE) return PICO_ERROR_GENERIC;

	return PICO_OK;
}

int bus_write_byte(uint8_t addr, uint8_t reg, uint8_t data)
{
	uint8_t buffer[2] = {reg, data};

	int r = i2c_write_timeout_us(BUS_DEVICE, addr, buffer, sizeof(buffer)/sizeof(buffer[0]), false, BUS_TIMEOUT);
	if (r < 0) return r;
	if (r != sizeof(buffer)/sizeof(buffer[0])) return PICO_ERROR_GENERIC;

	return PICO_OK;
}

int bus_read_block(uint8_t addr, uint8_t reg, uint8_t *dst, size_t len)
{
	int r = i2c_write_timeout_us(BUS_DEVICE, addr, &reg, BUS_SIZE_BYTE, false, BUS_TIMEOUT);
	if (r < 0) return r;
	if (r != BUS_SIZE_BYTE) return PICO_ERROR_GENERIC;

	r = i2c_read_timeout_us(BUS_DEVICE, addr, dst, len, false, BUS_TIMEOUT);
	if (r < 0) return r;
	if (r != len) return PICO_ERROR_GENERIC;

	return PICO_OK;
}

int bus_read_byte(uint8_t addr, uint8_t reg, uint8_t *dst)
{
	return bus_read_block(addr, reg, dst, BUS_SIZE_BYTE);
}

int bus_read_word(uint8_t addr, uint8_t reg, uint16_t *dst)
{
	return bus_read_block(addr, reg, (uint8_t *)dst, BUS_SIZE_WORD);
}

int bus_read_qword(uint8_t addr, uint8_t reg, uint64_t *dst)
{
	return bus_read_block(addr, reg, (uint8_t *)dst, BUS_SIZE_QWORD);
}
