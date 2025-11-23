#include <stdio.h>

#include "pico/binary_info.h"
#include "pico/stdlib.h"

#include "bus.h"

#define PROJ_DESC "I2CS Board RPi2 Example"

#define INTERVAL 400

#define SPL07_003_ADDR     0x77
#define SPL07_003_ADDR_ALT 0x76
#define ENS210_ADDR        0x43
#define APDS_9999_ADDR     0x52

int main()
{
	bi_decl(bi_program_description(PROJ_DESC));

	stdio_init_all();

	for (int i = 0; i < 10; i++)
	{
		sleep_ms(INTERVAL);
		putchar('.');
	}

	puts("\r\n" PROJ_DESC "\r\n");

	bus_setup();

	int r = bus_addr_check(SPL07_003_ADDR);
	printf("SPL07-003......: %s\r\n", bus_addr_check_to_str(r));

	r = bus_addr_check(SPL07_003_ADDR_ALT);
	printf("SPL07-003 (alt): %s\r\n", bus_addr_check_to_str(r));

	r = bus_addr_check(ENS210_ADDR);
	printf("ENS210.........: %s\r\n", bus_addr_check_to_str(r));

	r = bus_addr_check(APDS_9999_ADDR);
	printf("APDS-9999......: %s\r\n", bus_addr_check_to_str(r));

	while (1)
	{
		tight_loop_contents();
	}
}
