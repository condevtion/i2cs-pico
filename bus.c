#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/i2c.h"

#include "bus.h"

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

	return i2c_read_timeout_us(BUS_DEVICE, addr, &data, 1, false, BUS_TIMEOUT);
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

	return i2c_write_timeout_us(BUS_DEVICE, addr, buffer, sizeof(buffer)/sizeof(buffer[0]), false, BUS_TIMEOUT);
}

int bus_read_byte(uint8_t addr, uint8_t reg, uint8_t *dst)
{
	int r = i2c_write_timeout_us(BUS_DEVICE, addr, &reg, 1, false, BUS_TIMEOUT);
	if (r != 1) return r;

	return i2c_read_timeout_us(BUS_DEVICE, addr, dst, 1, false, BUS_TIMEOUT);
}

int bus_read_word(uint8_t addr, uint8_t reg, uint16_t *dst)
{
	int r = i2c_write_timeout_us(BUS_DEVICE, addr, &reg, 1, false, BUS_TIMEOUT);
	if (r != 1) return r;

	return i2c_read_timeout_us(BUS_DEVICE, addr, (uint8_t *)dst, 2, false, BUS_TIMEOUT);
}

int bus_read_qword(uint8_t addr, uint8_t reg, uint64_t *dst)
{
	int r = i2c_write_timeout_us(BUS_DEVICE, addr, &reg, 1, false, BUS_TIMEOUT);
	if (r != 1) return r;

	return i2c_read_timeout_us(BUS_DEVICE, addr, (uint8_t *)dst, 4, false, BUS_TIMEOUT);
}
