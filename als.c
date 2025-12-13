#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"

#include "bus.h"
#include "als.h"

const uint64_t _integration_times[] = {
	   400000, // 20-bit - 400 ms
	   200000, // 19-bit - 200 ms
	   100000, // 18-bit - 100 ms
	    50000, // 17-bit - 50 ms
	    25000, // 16-bit - 25 ms
	ALS_JIFFY  // 13-bit - 3.125 ms
};

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

int als_start_measure(uint8_t addr, bool rgb, uint8_t gain, uint8_t res, uint8_t rate, absolute_time_t *deadline)
{
	int r = bus_write_byte(addr, ALS_REG_LS_MEAS_RATE,
	                       ((res & ALS_MEAS_MASK) << 4) | (rate & ALS_MEAS_MASK));
	if (r != PICO_OK)
	{
		return r;
	}

	r = bus_write_byte(addr, ALS_REG_LS_GAIN, gain & ALS_MEAS_MASK);
	if (r != PICO_OK)
	{
		return r;
	}

	r = bus_write_byte(addr, ALS_REG_MAIN_CTRL,
	                   (rgb? ALS_MAIN_CTRL_RGB_MODE : 0) | ALS_MAIN_CTRL_LS_EN);
	if (r != PICO_OK)
	{
		return r;
	}

	if (deadline)
	{
		*deadline = delayed_by_us(get_absolute_time(),
		                          _integration_times[res & ALS_MEAS_MASK]);
	}

	return PICO_OK;
}

int als_check_result(uint8_t addr, uint8_t mask)
{
	int i = ALS_MAX_CHECKS;
	while (1)
	{
		uint8_t status;
		int r = bus_read_byte(addr, ALS_REG_MAIN_STATUS, &status);
		if (r != PICO_OK)
		{
			return r;
		}

		if (status & mask)
		{
			return PICO_OK;
		}

		if (--i <= 0)
		{
			break;
		}

		sleep_us(ALS_JIFFY);
	}

	return PICO_ERROR_TIMEOUT;
}

int als_read_al(uint8_t addr, uint32_t *value)
{
	uint8_t data[3];
	int r = bus_read_block(addr, ALS_REG_LS_DATA_GREEN, data, sizeof(data)/sizeof(data[0]));
	if (r != PICO_OK)
	{
		return r;
	}

	*value = ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[0];
	return PICO_OK;
}

int als_read_ir_rgb(uint8_t addr, uint32_t *ir, uint32_t *cr, uint32_t *cg, uint32_t *cb)
{
	uint8_t data[12];
	int r = bus_read_block(addr, ALS_REG_LS_DATA, data, sizeof(data)/sizeof(data[0]));
	if (r != PICO_OK)
	{
		return r;
	}

	*ir = ((uint32_t)data[2] << 16)  | ((uint32_t)data[1] << 8)  | (uint32_t)data[0];
	*cr = ((uint32_t)data[11] << 16) | ((uint32_t)data[10] << 8) | (uint32_t)data[9];
	*cg = ((uint32_t)data[5] << 16)  | ((uint32_t)data[4] << 8)  | (uint32_t)data[3];
	*cb = ((uint32_t)data[8] << 16)  | ((uint32_t)data[7] << 8)  | (uint32_t)data[6];
	return PICO_OK;
}

uint8_t als_get_gain(uint32_t al)
{
	// For illuminance above ~11520 lux set gain to 1x this allows to cover
	// range
	//    from 0.136 - 143092 lux at 20 bit resolution
	//    to 2.193 - 143719 at 16 bit
	if (al > 0xA00) return ALS_MEAS_GAIN_1;

	// 5760 lux - gain 3x
	//    0.045 - 47023 lux @ 20 bit
	//    0.722 - 47318 lux @ 16 bit
	if (al > 0x500) return ALS_MEAS_GAIN_3;

	// 3816 lux - gain 6x
	//    0.022 - 23501 lux @ 20 bit
	//    0.360 - 23608 lux @ 16 bit
	if (al > 0x350) return ALS_MEAS_GAIN_6;

	// 1872 lux - gain 9x
	//    0.007 - 7688 lux @ 20 bit
	//    0.117 - 7655 lux @ 16 bit
	if (al > 0x1A0) return ALS_MEAS_GAIN_9; //  1872 lux

	return ALS_MEAS_GAIN_18;
}

uint8_t als_get_gain_x(uint8_t gain)
{
	switch (gain)
	{
	case ALS_MEAS_GAIN_3:
		return 3;
	case ALS_MEAS_GAIN_6:
		return 6;
	case ALS_MEAS_GAIN_9:
		return 9;
	case ALS_MEAS_GAIN_18:
		return 18;
	}

	return 1;
}
