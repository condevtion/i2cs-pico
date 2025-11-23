#include <stdio.h>

#include "pico/binary_info.h"
#include "pico/stdlib.h"

#include "bus.h"
#include "prs.h"
#include "rhs.h"
#include "als.h"

#define PROJ_DESC "I2CS Board RPi2 Example"

#define INTERVAL 400

void report_sensor(const char *name, int r, const uint8_t *addr);

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

	puts("Enumerating sensors:\r");
	uint8_t prs_addr;
	report_sensor("SPL07-003", prs_find(&prs_addr), &prs_addr);

	uint8_t rhs_addr;
	report_sensor("ENS210...", rhs_find(&rhs_addr), &rhs_addr);

	uint8_t als_addr;
	report_sensor("APDS-9999", als_find(&als_addr), &als_addr);

	while (1)
	{
		tight_loop_contents();
	}
}

void report_sensor(const char *name, int r, const uint8_t *addr)
{
	if (r < 0)
	{
		printf("\t%s: %s\r\n", name, bus_addr_check_to_str(r));
	}
	else
	{
		printf("\t%s: 0x%02hhx\r\n", name, *addr);
	}
}
