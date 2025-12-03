#include <stdint.h>
#include <stdbool.h>

#include "pico/error.h"
#include "pico/stdlib.h"

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

int prs_check_coefs(uint8_t addr, bool *rdy)
{
	uint8_t data;
	int r = bus_read_byte(addr, PRS_MEAS_CFG, &data);
	if (r != PICO_OK)
	{
		return r;
	}

	*rdy = data & PRS_COEF_RDY;

	return PICO_OK;
}

int prs_wait_coefs(uint8_t addr, bool *rdy)
{
	int i = 4;
	while(1)
	{
		int r = prs_check_coefs(addr, rdy);
		if (r != PICO_OK || *rdy || --i <= 0)
		{
			return r;
		}

		sleep_ms(PRS_COEF_JIFFY);
	}

	return PICO_OK;
}

int prs_read_coefs(uint8_t addr, prs_coefs_t *coefs)
{
	uint8_t raw_coefs[PRS_COEF_SIZE];

	int r = bus_read_block(addr, RRS_COEF, raw_coefs, sizeof(raw_coefs)/sizeof(raw_coefs[0]));
	if (r != PICO_OK)
	{
		return r;
	}

	coefs->c0 = ((short)raw_coefs[0] << 4) | ((short)raw_coefs[1] >> 4);
	coefs->c1 = (((short)raw_coefs[1] & 0xf) << 8) | (short)raw_coefs[2];
	coefs->c00 = ((int)raw_coefs[3] << 12) | ((int)raw_coefs[4] << 4) | ((int)raw_coefs[5] >> 4);
	coefs->c10 = (((int)raw_coefs[5] & 0xf) << 16) | ((int)raw_coefs[6] << 8) | (int)raw_coefs[7];
	coefs->c01 = ((short)raw_coefs[8] << 8) | (short)raw_coefs[9];
	coefs->c11 = ((short)raw_coefs[10] << 8) | (short)raw_coefs[11];
	coefs->c20 = ((short)raw_coefs[12] << 8) | (short)raw_coefs[13];
	coefs->c21 = ((short)raw_coefs[14] << 8) | (short)raw_coefs[15];
	coefs->c30 = ((short)raw_coefs[16] << 8) | (short)raw_coefs[17];
	coefs->c31 = ((short)raw_coefs[18] << 4) | ((short)raw_coefs[19] >> 4);
	coefs->c40 = (((short)raw_coefs[19] & 0xf) << 8) | (short)raw_coefs[20];

	return PICO_OK;
}
