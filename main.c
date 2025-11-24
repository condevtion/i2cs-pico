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
void halt_if_no_devices(uint8_t prs_addr, uint8_t rhs_addr, uint8_t als_addr);
void enumerate_sensors(uint8_t *prs_addr, uint8_t *rhs_addr, uint8_t *als_addr);
void check_sensors(uint8_t *prs_addr, uint8_t *rhs_addr, uint8_t *als_addr);
void check_prs_id(uint8_t *addr);
void check_rhs_id(uint8_t *addr);
void check_als_id(uint8_t *addr);

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

	uint8_t prs_addr, rhs_addr, als_addr;
	enumerate_sensors(&prs_addr, &rhs_addr, &als_addr);
	check_sensors(&prs_addr, &rhs_addr, &als_addr);


	while (1)
	{
		tight_loop_contents();
	}
}

void report_sensor(const char *name, int r, const uint8_t *addr)
{
	if (r < 0)
	{
		printf("\t%s: %s\r\n", name, bus_error_to_str(r));
	}
	else
	{
		printf("\t%s: 0x%02hhx\r\n", name, *addr);
	}
}

void halt_if_no_devices(uint8_t prs_addr, uint8_t rhs_addr, uint8_t als_addr)
{
	if (prs_addr > BUS_ADDR_MAX && rhs_addr > BUS_ADDR_MAX && als_addr > BUS_ADDR_MAX)
	{
		puts("\nNo compatible devices found. Stopping...\r");
		while (1)
		{
			tight_loop_contents();
		}
	}
}

void enumerate_sensors(uint8_t *prs_addr, uint8_t *rhs_addr, uint8_t *als_addr)
{
	puts("Enumerating sensors:\r");
	report_sensor("SPL07-003", prs_find(prs_addr), prs_addr);
	report_sensor("ENS210...", rhs_find(rhs_addr), rhs_addr);
	report_sensor("APDS-9999", als_find(als_addr), als_addr);

	halt_if_no_devices(*prs_addr, *rhs_addr, *als_addr);
}

void check_sensors(uint8_t *prs_addr, uint8_t *rhs_addr, uint8_t *als_addr)
{
	puts("\nChecking sensor IDs:\r");
	if (*prs_addr <= BUS_ADDR_MAX) check_prs_id(prs_addr);
	if (*rhs_addr <= BUS_ADDR_MAX) check_rhs_id(rhs_addr);
	if (*als_addr <= BUS_ADDR_MAX) check_als_id(als_addr);

	halt_if_no_devices(*prs_addr, *rhs_addr, *als_addr);
}

void check_prs_id(uint8_t *addr)
{
	uint8_t id;
	int r = prs_get_id(*addr, &id);
	if (r != PICO_OK)
	{
		*addr = BUS_ADDR_INVALID;
		printf("\tSPL07-003 ID: %s. Ignoring device...\r\n", bus_error_to_str(r));
	}
	else if (id != PRS_ID)
	{
		*addr = BUS_ADDR_INVALID;
		printf("\tSPL07-003 ID: %hhu.%hhu (invalid, expected %hhu.%hhu). Ignoring device...\r\n",
		       id & 0x0f, (id & 0xf0) >> 4,
		       PRS_ID & 0x0f, (PRS_ID & 0xf0) >> 4);
	}
	else
	{
		printf("\tSPL07-003 ID: %hhu.%hhu\r\n", id & 0x0f, (id & 0xf0) >> 4);
	}
}

void check_rhs_id(uint8_t *addr)
{
	uint16_t id, rev;
	uint64_t uid;

	int r = rhs_get_id(*addr, &id, &rev, &uid);
	if (r != PICO_OK)
	{
		*addr = BUS_ADDR_INVALID;
		printf("\tENS210 ID...: %s. Ignoring device...\r\n", bus_error_to_str(r));
	}
	else if (id != RHS_ID || rev != RHS_REV)
	{
		*addr = BUS_ADDR_INVALID;
		printf("\tENS210 ID...: %hx.%hu (invalid, expected %hx.%hu). Ignoring device...\r\n",
		       id, rev, RHS_ID, RHS_REV);
	}
	else
	{
		printf("\tENS210 ID...: %hx.%hu (%016llX)\r\n", id, rev, uid);
	}
}

void check_als_id(uint8_t *addr)
{
	uint8_t id;
	int r = als_get_id(*addr, &id);
	if (r != PICO_OK)
	{
		*addr = BUS_ADDR_INVALID;
		printf("\tAPDS-9999 ID: %s. Ignoring device...\r\n", bus_error_to_str(r));
	}
	else if (id != ALS_ID)
	{
		*addr = BUS_ADDR_INVALID;
		printf("\tAPDS-9999 ID: %hhu.%hhu (invalid, expected %hhu.%hhu). Ignoring device...\r\n",
		       (id & 0xf0) >> 4, id & 0x0f,
		       (ALS_ID & 0xf0) >> 4, ALS_ID & 0x0f);
	}
	else
	{
		printf("\tAPDS-9999 ID: %hhu.%hhu\r\n",
		       (id & 0xf0) >> 4, id & 0x0f);
	}
}
