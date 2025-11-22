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

const char *bus_addr_check(uint8_t addr)
{
	uint8_t data;

	switch (i2c_read_timeout_us(i2c_default, addr, &data, 1, false, BUS_TIMEOUT))
	{
	case PICO_ERROR_GENERIC:
		return "no";
	case PICO_ERROR_TIMEOUT:
		return "timeout";
	}

	return "yes";
}
