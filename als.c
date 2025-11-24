#include <stdint.h>

#include "pico/error.h"

#include "bus.h"
#include "als.h"

int als_find(uint8_t *addr)
{
	int r = bus_addr_check(ALS_ADDR);
	if (r != PICO_OK)
	{
		*addr = BUS_ADDR_INVALID;
		return r;
	}

	*addr = ALS_ADDR;
	return PICO_OK;
}

int als_get_id(uint8_t addr, uint8_t *id)
{
	uint8_t data;
	int r = bus_read_byte(addr, ALS_REG_ID, &data);
	if (r != PICO_OK)
	{
		return r;
	}

	*id = data;
	return PICO_OK;
}
