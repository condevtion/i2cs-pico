#include <stdio.h>

#include "pico/binary_info.h"
#include "pico/stdlib.h"

#include "hardware/i2c.h"

#define PROJ_DESC "I2CS Board RPi2 Example"

#define INTERVAL 400

#define SPL07_003_ADDR     0x77
#define SPL07_003_ADDR_ALT 0x76
#define ENS210_ADDR        0x43
#define APDS_9999_ADDR     0x52

const char* i2c_addr_check(uint8_t addr)
{
	uint8_t data;

	switch (i2c_read_timeout_us(i2c_default, addr, &data, 1, false, 100000))
	{
	case PICO_ERROR_GENERIC:
		return "no";
	case PICO_ERROR_TIMEOUT:
		return "timeout";
	}

	return "yes";
}

int main()
{
	bi_decl(bi_program_description(PROJ_DESC));
	bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

	stdio_init_all();

	for (int i = 0; i < 10; i++)
	{
		sleep_ms(INTERVAL);
		putchar('.');
	}

	puts("\r\n" PROJ_DESC "\r\n");

	i2c_init(i2c_default, 100 * 1000);
	gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
	gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
	gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
	gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

	printf("SPL07-003......: %s\r\n", i2c_addr_check(SPL07_003_ADDR));
	printf("SPL07-003 (alt): %s\r\n", i2c_addr_check(SPL07_003_ADDR_ALT));
	printf("ENS210.........: %s\r\n", i2c_addr_check(ENS210_ADDR));
	printf("APDS-9999......: %s\r\n", i2c_addr_check(APDS_9999_ADDR));

	while (1)
	{
		tight_loop_contents();
	}
}
