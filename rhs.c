#include <stdint.h>

#include "bus.h"
#include "rhs.h"

int rhs_find(uint8_t *addr)
{
	int r = bus_addr_check(RHS_ADDR);
	if (r < 0)
	{
		*addr = 0;
		return r;
	}

	*addr = RHS_ADDR;
	return 0;
}
