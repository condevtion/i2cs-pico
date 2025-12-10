#include <stdint.h>

#include "pico/error.h"
#include "pico/time.h"

#include "bus.h"
#include "rhs.h"

int rhs_find(uint8_t *addr)
{
	int r = bus_addr_check(RHS_ADDR);
	if (r != PICO_OK)
	{
		*addr = BUS_ADDR_INVALID;
		return r;
	}

	*addr = RHS_ADDR;
	return PICO_OK;
}

int rhs_activate(uint8_t addr)
{
	int r = bus_write_byte(addr, RHS_REG_SYS_CTRL, 0);
	if (r != PICO_OK)
	{
		return r;
	}

	uint8_t data;
	for (int i = 0; i < 3; i++)
	{
		sleep_us(RHS_T_BOOTING);
		r = bus_read_byte(addr, RHS_REG_SYS_STAT, &data);
		if (r != PICO_OK)
		{
			return r;
		}

		if (data & RHS_SYS_STAT_ACTIVE)
		{
			return PICO_OK;
		}
	}

	return PICO_ERROR_TIMEOUT;
}

int rhs_deactivate(uint8_t addr)
{
	return bus_write_byte(addr, RHS_REG_SYS_CTRL, RHS_SYS_CTRL_LOW_POWER);
}

int rhs_get_id(uint8_t addr, uint16_t *id, uint16_t *rev, uint64_t *uid)
{
	int r = rhs_activate(addr);
	if (r != PICO_OK)
	{
		return r;
	}

	r = bus_read_word(addr, RHS_REG_PART_ID, id);
	if (r != PICO_OK)
	{
		rhs_deactivate(addr);
		return r;
	}

	r = bus_read_word(addr, RHS_REG_DIE_REV, rev);
	if (r != PICO_OK)
	{
		rhs_deactivate(addr);
		return r;
	}

	if (uid)
	{
		r = bus_read_qword(addr, RHS_REG_UID, uid);
		if (r != PICO_OK)
		{
			rhs_deactivate(addr);
			return r;
		}
	}

	return rhs_deactivate(addr);
}

#define _DATA7_WIDTH 17
#define _DATA7_MASK  ((1<<_DATA7_WIDTH) - 1)
#define _DATA7_MSB   (1<<(_DATA7_WIDTH-1))

#define _CRC7_WIDTH  7
#define _CRC7_POLY   0x89
#define _CRC7_IVEC   0x7f

#define _DATA7_MASK_HIGH (_DATA7_MASK<<_CRC7_WIDTH)
#define _CRC7_IPOL       (_CRC7_POLY<<(_DATA7_WIDTH-1))
#define _CRC7_IBIT       (_DATA7_MSB<<_CRC7_WIDTH)

uint8_t rhs_crc7(uint32_t x)
{
	x = ((x & _DATA7_MASK) << _CRC7_WIDTH) | _CRC7_IVEC;
	uint32_t p = _CRC7_IPOL, b = _CRC7_IBIT;
	while (b & _DATA7_MASK_HIGH)
	{
		if (b & x) x ^= p;
		p >>= 1;
		b >>= 1;
	}
	return x;
}

int rhs_start(uint8_t addr)
{
	return bus_write_byte(addr, RHS_REG_SENS_START, RHS_H_START | RHS_T_START);
}

int rhs_read_values(uint8_t addr,
                    uint32_t *t_data, bool *t_valid, uint8_t *t_crc,
                    uint32_t *h_data, bool *h_valid, uint8_t *h_crc)
{
	uint8_t data[6];
	int r = bus_read_block(addr, RHS_REG_T_VAL, data, sizeof(data)/sizeof(data[0]));
	if (r != PICO_OK)
	{
		return r;
	}

	*t_data = (uint32_t)data[0] | ((uint32_t)data[1]<<8);
	*t_valid = data[2] & 1;
	*t_crc = (data[2]>>1) & 0x7f;

	*h_data = (uint32_t)data[3] | ((uint32_t)data[4]<<8);
	*h_valid = data[5] & 1;
	*h_crc = (data[5]>>1) & 0x7f;

	return PICO_OK;
}
