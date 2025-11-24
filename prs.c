#include <stdint.h>

#include "pico/error.h"

#include "bus.h"
#include "prs.h"

int prs_find(uint8_t *addr)
{
	int r = bus_addr_check(PRS_ADDR);
	if (r != PICO_OK)
	{
		if (bus_addr_check(PRS_ADDR_ALT) != PICO_OK)
		{
			*addr = BUS_ADDR_INVALID;
			return r;
		}

		*addr = PRS_ADDR_ALT;
	}
	else
	{
		*addr = PRS_ADDR;
	}

	return PICO_OK;
}

int prs_get_id(uint8_t addr, uint8_t *id)
{
	uint8_t data;
	int r = bus_read_byte(addr, PRS_REG_ID, &data);
	if (r != PICO_OK)
	{
		return r;
	}

	*id = data;
	return PICO_OK;
}
