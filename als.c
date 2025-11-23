#include <stdint.h>

#include "bus.h"
#include "als.h"

int als_find(uint8_t *addr)
{
	int r = bus_addr_check(ALS_ADDR);
	if (r < 0)
	{
		*addr = 0;
		return r;
	}

	*addr = ALS_ADDR;
	return 0;
}
