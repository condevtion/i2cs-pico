#include <stdint.h>

#include "bus.h"
#include "prs.h"

int prs_find(uint8_t *addr)
{
	int r = bus_addr_check(PRS_ADDR);
	if (r < 0)
	{
		if (bus_addr_check(PRS_ADDR_ALT) < 0)
		{
			*addr = 0;
			return r;
		}

		*addr = PRS_ADDR_ALT;
	}
	else
	{
		*addr = PRS_ADDR;
	}

	return 0;
}
