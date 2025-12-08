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

int16_t _u12_to_s(uint16_t u)
{
	return (u & 0x0800)?  -2048 + (u & 0x07ff) : u;
}

int16_t _u16_to_s(uint16_t u)
{
	return (u & 0x8000)? -32768 + (u & 0x7fff) : u;
}

int32_t _u20_to_s(uint32_t u)
{
	return (u & 0x00080000)?  -524288 + (u & 0x0007ffff) : u;
}

int32_t _u24_to_s(uint32_t u)
{
	return (u & 0x00800000)? -8388608 + (u & 0x007fffff) : u;
}

int prs_read_coefs(uint8_t addr, prs_coefs_t *coefs)
{
	uint8_t raw_coefs[PRS_COEF_SIZE];

	int r = bus_read_block(addr, RRS_COEF, raw_coefs, sizeof(raw_coefs)/sizeof(raw_coefs[0]));
	if (r != PICO_OK)
	{
		return r;
	}

	coefs->c0 = _u12_to_s(((uint16_t)raw_coefs[0] << 4) | ((uint16_t)raw_coefs[1] >> 4));
	coefs->c1 = _u12_to_s((((uint16_t)raw_coefs[1] & 0xf) << 8) | (uint16_t)raw_coefs[2]);
	coefs->c00 = _u20_to_s(((uint32_t)raw_coefs[3] << 12) |
	                       ((uint32_t)raw_coefs[4] << 4) |
	                       ((uint32_t)raw_coefs[5] >> 4));
	coefs->c10 = _u20_to_s((((uint32_t)raw_coefs[5] & 0xf) << 16) |
	                       ((uint32_t)raw_coefs[6] << 8) |
	                       (uint32_t)raw_coefs[7]);
	coefs->c01 = _u16_to_s(((uint16_t)raw_coefs[8] << 8) | (uint16_t)raw_coefs[9]);
	coefs->c11 = _u16_to_s(((uint16_t)raw_coefs[10] << 8) | (uint16_t)raw_coefs[11]);
	coefs->c20 = _u16_to_s(((uint16_t)raw_coefs[12] << 8) | (uint16_t)raw_coefs[13]);
	coefs->c21 = _u16_to_s(((uint16_t)raw_coefs[14] << 8) | (uint16_t)raw_coefs[15]);
	coefs->c30 = _u16_to_s(((uint16_t)raw_coefs[16] << 8) | (uint16_t)raw_coefs[17]);
	coefs->c31 = _u12_to_s(((uint16_t)raw_coefs[18] << 4) | ((uint16_t)raw_coefs[19] >> 4));
	coefs->c40 = _u12_to_s((((uint16_t)raw_coefs[19] & 0xf) << 8) | (uint16_t)raw_coefs[20]);

	return PICO_OK;
}

int32_t prs_prc_to_k(uint8_t prc)
{
	switch (prc)
	{
	case PRS_PRC_1:
		return 524288;  //0x00080000
	case PRS_PRC_2:
		return 1572864; //0x00180000
	case PRS_PRC_4:
		return 3670016; //0x00380000
	case PRS_PRC_8:
		return 7864320; //0x00780000
	case PRS_PRC_16:
		return 253952;  //0x0003e000
	case PRS_PRC_32:
		return 516096;  //0x0007e000
	case PRS_PRC_64:
		return 1040384; //0x000fe000
	case PRS_PRC_128:
		return 2088960; //0x001fe000
	}

	return -1;
}

int prs_config(uint8_t addr, uint8_t pm_prc, uint8_t tmp_prc)
{
	int r = bus_write_byte(addr, PRS_PRS_CFG, PRS_RATE_1_MPS | (pm_prc & PRS_PRC_MASK));
	if (r != PICO_OK)
	{
		return r;
	}

	r = bus_write_byte(addr, PRS_TMP_CFG, PRS_RATE_1_MPS | (tmp_prc & PRS_PRC_MASK));
	if (r != PICO_OK)
	{
		return r;
	}

	uint8_t cfg = PRS_CFG_INT_PRS | PRS_CFG_INT_TMP;

	if (pm_prc & PRS_PRC_SHIFT_CHK)
	{
		cfg |= PRS_CFG_P_SHIFT;
	}

	if (tmp_prc & PRS_PRC_SHIFT_CHK)
	{
		cfg |= PRS_CFG_T_SHIFT;
	}

	return bus_write_byte(addr, PRS_CFG_REG, cfg);
}

int prs_meas_config(uint8_t addr)
{
	return bus_write_byte(addr, PRS_MEAS_CFG, PRS_MEAS_CTRL_CPT);
}

int prs_meas_stop(uint8_t addr)
{
	return bus_write_byte(addr, PRS_MEAS_CFG, 0);
}

int _get_raw(uint8_t addr, uint8_t reg, int32_t *value)
{
	uint8_t data[3];
	int r = bus_read_block(addr, reg, data, sizeof(data)/sizeof(data[0]));
	if (r != PICO_OK)
	{
		return r;
	}

	*value = _u24_to_s(((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[2]);
	return PICO_OK;
}

int prs_get_prs_raw(uint8_t addr, int32_t *prs)
{
	return _get_raw(addr, PRS_PRS, prs);
}

int prs_get_tmp_raw(uint8_t addr, int32_t *tmp)
{
	return _get_raw(addr, PRS_TMP, tmp);
}
