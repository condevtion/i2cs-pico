#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/i2c.h"

#define BUS_FREQ    (100 * 1000) // 100 kHz
#define BUS_TIMEOUT (100 * 1000) // 100 ms

void bus_setup(void)
{
	bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

	i2c_init(i2c_default, BUS_FREQ);
	gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
	gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
	gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
	gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
}

int bus_addr_check(uint8_t addr)
{
	uint8_t data;

	return i2c_read_timeout_us(i2c_default, addr, &data, 1, false, BUS_TIMEOUT);
}

const char *bus_addr_check_to_str(int result)
{
	switch (result)
	{
	case PICO_ERROR_GENERIC:
		return "error";
	case PICO_ERROR_TIMEOUT:
		return "timeout";
	case 1:
		return "ok";
	}

	return "unknown";
}

int bus_write_byte(uint8_t addr, uint8_t reg, uint8_t data)
{
	uint8_t buffer[2] = {reg, data};

	return i2c_write_timeout_us(i2c_default, addr, buffer, sizeof(buffer)/sizeof(uint8_t), false, BUS_TIMEOUT);
}

int bus_read_byte(uint8_t addr, uint8_t reg, uint8_t *dst)
{
	int r = i2c_write_timeout_us(i2c_default, addr, &reg, 1, false, BUS_TIMEOUT);
	if (r != 1) return r;

	return i2c_read_timeout_us(i2c_default, addr, dst, 1, false, BUS_TIMEOUT);
}

int bus_read_word(uint8_t addr, uint8_t reg, uint16_t *dst)
{
	int r = i2c_write_timeout_us(i2c_default, addr, &reg, 1, false, BUS_TIMEOUT);
	if (r != 1) return r;

	return i2c_read_timeout_us(i2c_default, addr, (uint8_t *)dst, 2, false, BUS_TIMEOUT);
}

int bus_read_qword(uint8_t addr, uint8_t reg, uint64_t *dst)
{
	int r = i2c_write_timeout_us(i2c_default, addr, &reg, 1, false, BUS_TIMEOUT);
	if (r != 1) return r;

	return i2c_read_timeout_us(i2c_default, addr, (uint8_t *)dst, 4, false, BUS_TIMEOUT);
}
