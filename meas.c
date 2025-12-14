#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"

#include "bus.h"
#include "prs.h"
#include "rhs.h"
#include "als.h"

#include "meas.h"

void int_setup(uint gpio)
{
	gpio_init(gpio);
	gpio_set_dir(gpio,GPIO_IN);
	gpio_pull_up(gpio);
}

void setup_prs(uint8_t addr, prs_coefs_t *coefs, int32_t *prs_k, int32_t *tmp_k)
{
	if (addr > BUS_ADDR_MAX) return;

	puts("\nSPL07-003 Calibration Coefficients:\r");

	bool rdy = false;
	int r = prs_wait_coefs(addr, &rdy);
	if (r != PICO_OK)
	{
		printf("\terror while waiting for the coefficients: %d\r\n", r);
		return;
	}
	if (!rdy)
	{
		puts("\tthe coefficients aren't ready\r");
		return;
	}

	r = prs_read_coefs(addr, coefs);
	if (r != PICO_OK)
	{
		printf("\terror while reading the coefficients: %d\r\n", r);
	}

	printf("\tc0: %+8hd, c00: %+8d, c01: %+8hd\r\n"     \
	       "\tc1: %+8hd, c10: %+8d, c11: %+8hd\r\n"     \
	       "\t              c20: %+8hd, c21: %+8hd\r\n" \
	       "\t              c30: %+8hd, c31: %+8hd\r\n" \
	       "\t              c40: %+8hd\r\n",
	       coefs->c0, coefs->c00, coefs->c01,
	       coefs->c1, coefs->c10, coefs->c11,
	       coefs->c20, coefs->c21,
	       coefs->c30, coefs->c31,
	       coefs->c40);

	*prs_k = prs_prc_to_k(PRS_PRC_16);
	*tmp_k = prs_prc_to_k(PRS_PRC_1);

	printf("\nConfiguring pressure sensor (p scale %d, t scale %d)...", *prs_k, *tmp_k);
	r = prs_config(addr, PRS_PRC_16, PRS_PRC_1);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n", r);
		return;
	}
	puts(" ok\r");
}

void _clr_int(uint8_t addr)
{
	printf("Interrupt status:");
	uint8_t sts;
	int r = bus_read_byte(addr, PRS_INT_STS, &sts);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n");
	}
	else
	{
		printf(" INT_PRS: %c, INT_TMP: %c, INT_FIFO: %c\r\n",
		       (sts & PRS_INT_PRS)? 'Y':'N',
		       (sts & PRS_INT_TMP)? 'Y':'N',
		       (sts & PRS_INT_FIFO_FULL)? 'Y':'N');
	}

	printf("Interrupt pin(%d): %d\r\n", PRS_INT_PIN, gpio_get(PRS_INT_PIN));
}

void _read_prs_raw(uint8_t addr, int32_t k, float *raw_sc)
{
	printf("Reading raw pressure value...");
	int32_t raw;
	int r = prs_get_prs_raw(addr, &raw);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n");
	}
	else
	{
		*raw_sc = (float)raw/k;
		printf(" p_raw: %d, p_raw_sc: %.7f\r\n", raw, *raw_sc);
	}
}

void _read_tmp_raw(uint8_t addr, int32_t k, float *raw_sc)
{
	printf("Reading raw temperature value from pressure sensor...");
	int32_t raw;
	int r = prs_get_tmp_raw(addr, &raw);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n");
	}
	else
	{
		*raw_sc = (float)raw/k;
		printf(" t_raw: %d, t_raw_sc: %.7f\r\n", raw, *raw_sc);
	}
}

void start_measure_prs(uint8_t addr, absolute_time_t *start)
{
	if (addr > BUS_ADDR_MAX) return;

	printf("Interrupt pin(%d): %d\r\n", PRS_INT_PIN, gpio_get(PRS_INT_PIN));

	printf("Starting pressure measurement...");
	int r = prs_meas_config_prs(addr);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n", r);
		return;
	}
	puts(" ok\r");

	if (start)
	{
		*start = get_absolute_time();
	}
}

void start_measure_prs_tmp(uint8_t addr, absolute_time_t *start)
{
	if (addr > BUS_ADDR_MAX) return;

	printf("Interrupt pin(%d): %d\r\n", PRS_INT_PIN, gpio_get(PRS_INT_PIN));

	printf("Starting temperature measurement with pressure sensor...");
	int r = prs_meas_config_tmp(addr);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n", r);
		return;
	}
	puts(" ok\r");

	if (start)
	{
		*start = get_absolute_time();
	}
}

void read_prs_raw_data(uint8_t addr, int32_t k, float *p_raw_sc, absolute_time_t start)
{
	if (addr > BUS_ADDR_MAX || is_nil_time(start)) return;

	printf("SPL07-003");
	for (size_t i=1; i < 4166667; i++)
	{
		if (!gpio_get(PRS_INT_PIN))
		{
			absolute_time_t end = get_absolute_time();
			printf(" - done (int@%d - 0, %.3f s): %lld us (%d clk)\r\n",
			       PRS_INT_PIN,
			       (float)to_ms_since_boot(end)/1000,
			       absolute_time_diff_us(start, end),
			       i);

			printf("Reading pressure sensor operating status...");
			uint8_t cfg = 0;
			int r = bus_read_byte(addr, PRS_MEAS_CFG, &cfg);
			if (r != PICO_OK)
			{
				printf(" error: %d\r\n", r);
				return;
			}
			printf(" PRS_RDY: %c, TMP_RDY: %c\r\n",
			       (cfg & PRS_PRS_RDY)? 'Y':'N',
			       (cfg & PRS_TMP_RDY)? 'Y':'N');

			_clr_int(addr);

			if (cfg & PRS_PRS_RDY)
			{
				_read_prs_raw(addr, k, p_raw_sc);
			}

			return;
		}

		sleep_us(PRS_JIFFY);

		if (!(i%28800))
		{
			puts(".\r");
		}
		else if (!(i%72))
		{
			putchar('.');
		}
	}
}


void read_prs_tmp_raw_data(uint8_t addr, int32_t k, float *t_raw_sc, absolute_time_t start)
{
	if (addr > BUS_ADDR_MAX || is_nil_time(start)) return;

	printf("SPL07-003");
	for (size_t i=1; i < 4166667; i++)
	{
		if (!gpio_get(PRS_INT_PIN))
		{
			absolute_time_t end = get_absolute_time();
			printf(" - done (int@%d - 0, %.3f s): %lld us (%d clk)\r\n",
			       PRS_INT_PIN,
			       (float)to_ms_since_boot(end)/1000,
			       absolute_time_diff_us(start, end),
			       i);

			printf("Reading pressure sensor operating status...");
			uint8_t cfg = 0;
			int r = bus_read_byte(addr, PRS_MEAS_CFG, &cfg);
			if (r != PICO_OK)
			{
				printf(" error: %d\r\n", r);
				return;
			}
			printf(" PRS_RDY: %c, TMP_RDY: %c\r\n",
			       (cfg & PRS_PRS_RDY)? 'Y':'N',
			       (cfg & PRS_TMP_RDY)? 'Y':'N');

			_clr_int(addr);

			if (cfg & PRS_TMP_RDY)
			{
				_read_tmp_raw(addr, k, t_raw_sc);
			}

			return;
		}

		sleep_us(PRS_JIFFY);

		if (!(i%28800))
		{
			puts(".\r");
		}
		else if (!(i%72))
		{
			putchar('.');
		}
	}
}

void calc_prs(uint8_t addr, const prs_coefs_t *coefs, float t_raw_sc, float p_raw_sc)
{
	if (addr > BUS_ADDR_MAX) return;

	if (isnan(t_raw_sc))
	{
		puts("SPL07-003 - T: NaN, P: NaN\r");
		return;
	}

	float t = coefs->c0*0.5 + coefs->c1 * t_raw_sc;
	if (isnan(p_raw_sc))
	{
		printf("SPL07-003 - T: %+.1f C, P: NaN\r\n", t);
		return;
	}

	float p = p_raw_sc;
	float p2 = p * p;
	float p3 = p2 * p;
	float p4 = p3 * p;
	float pt_corr = t_raw_sc*(coefs->c01 + coefs->c11*p + coefs->c21*p2 + coefs->c31*p3);
	p = pt_corr + coefs->c00 + coefs->c10*p + coefs->c20*p2 + coefs->c30*p3 + coefs->c40*p4;
	printf("SPL07-003 - T: %+.1f C, P: %.2f mbar\r\n", t, p/100);
}

void start_measure_rhs(uint8_t addr, absolute_time_t *deadline, absolute_time_t *start)
{
	if (addr > BUS_ADDR_MAX) return;

	printf("Starting relative humidity measurement...");
	int r = rhs_start(addr);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n", r);

		if (deadline)
		{
			*deadline = nil_time;
		}
		return;
	}
	puts(" ok\r");

	absolute_time_t _start = get_absolute_time();
	if (start)
	{
		*start = _start;
	}

	if (deadline)
	{
		*deadline = delayed_by_us(_start, RHS_T_CONV_TH_SINGLE);
	}
}

void read_rhs_data(uint8_t addr, absolute_time_t deadline, absolute_time_t start)
{
	if (addr > BUS_ADDR_MAX || is_nil_time(deadline)) return;

	sleep_until(deadline);
	absolute_time_t end = get_absolute_time();
	printf("ENS210 - done(%.3f s): %lld us\r\n",
	       (float)to_ms_since_boot(end)/1000,
	       absolute_time_diff_us(start, end));

	printf("Reading relative humidity sensor data...");
	uint32_t t_data, h_data;
	bool t_valid, h_valid;
	uint8_t t_crc, h_crc;
	int r = rhs_read_values(addr,
	                        &t_data, &t_valid, &t_crc,
	                        &h_data, &h_valid, &h_crc);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n", r);
		return;
	}
	puts(" ok\r");

	float t = (float)t_data / 64 - 273.15;
	printf("ENS210 T.: %+.2f C (0x%04hx), valid: %c, CRC: 0x%02hhx",
	       t, t_data, t_valid? 'Y': 'N', t_crc);
	uint8_t calc_crc = rhs_crc7(t_data + (t_valid? 0x10000:0));
	if (calc_crc == t_crc)
	{
		puts(" (ok)\r");
	}
	else
	{
		printf(" (fail: 0x%02hhx)\r\n", calc_crc);
	}

	float h = (float)h_data / 512;
	printf("ENS210 RH:  %.1f %%  (0x%04hx), valid: %c, CRC: 0x%02hhx",
	       h, h_data, h_valid? 'Y': 'N', h_crc);
	calc_crc = rhs_crc7(h_data + (h_valid? 0x10000:0));
	if (calc_crc == h_crc)
	{
		puts(" (ok)\r");
	}
	else
	{
		printf(" (fail: 0x%02hhx)\r\n", calc_crc);
	}
}

void probe_als(uint8_t addr, uint8_t *gain)
{
	if (addr > BUS_ADDR_MAX) return;

	printf("Starting ambient light probing...");
	absolute_time_t deadline, start;
	int r = als_start_measure(addr, false, ALS_MEAS_GAIN_1, ALS_MEAS_RES_13, ALS_MEAS_RATE_2000,
	                          &deadline, &start);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n", r);
		return;
	}
	puts(" ok\r");

	sleep_until(deadline);
	r = als_check_result(addr, ALS_STATUS_LS_DATA);
	absolute_time_t end = get_absolute_time();
	if (r != PICO_OK)
	{
		printf(" fail(%.3f s|%lld us): %d\r\n",
		       (float)to_ms_since_boot(end)/1000,
		       absolute_time_diff_us(start, end),
		       r);
		return;
	}
	printf("APDS-9999 - done(%.3f s): %lld us\r\n",
	       (float)to_ms_since_boot(end)/1000,
	       absolute_time_diff_us(start, end));

	printf("Reading preliminary ambient light sensor data...");
	uint32_t al;
	r = als_read_al(addr, &al);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n", r);
		return;
	}
	puts(" ok\r");

	uint8_t _gain = als_get_gain(al);
	printf("APDS-9999 (probe): AL: %.1f lux (0x%06lu), gain: %hhdx\r\n",
	       (float)al * 11.0, al, als_get_gain_x(_gain));

	if (gain)
	{
		*gain = _gain;
	}
}

void start_measure_als(uint8_t addr, bool rgb, uint8_t gain, uint8_t res,
                       absolute_time_t *deadline, absolute_time_t *start)
{
	if (addr > BUS_ADDR_MAX) return;

	printf("Starting %s measurement...", rgb? "color" : "ambient light");
	int r = als_start_measure(addr, rgb, gain, res, ALS_MEAS_RATE_2000,
	                          deadline, start);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n", r);
		return;
	}
	puts(" ok\r");
}

void read_als_light_data(uint8_t addr, uint8_t gain, uint8_t res,
                         absolute_time_t deadline, absolute_time_t start)
{
	if (addr > BUS_ADDR_MAX || is_nil_time(deadline)) return;

	sleep_until(deadline);

	int r = als_check_result(addr, ALS_STATUS_LS_DATA);
	absolute_time_t end = get_absolute_time();
	if (r != PICO_OK)
	{
		printf(" fail(%.3f s|%lld us): %d\r\n",
		       (float)to_ms_since_boot(end)/1000,
		       absolute_time_diff_us(start, end),
		       r);
		return;
	}
	printf("APDS-9999 - done(%.3f s): %lld us\r\n",
	       (float)to_ms_since_boot(end)/1000,
	       absolute_time_diff_us(start, end));

	printf("Reading ambient light data...");
	uint32_t al;
	r = als_read_al(addr, &al);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n", r);
		return;
	}
	puts(" ok\r");

	float scale = als_get_scale(gain, res);
	printf("APDS-9999 AL: %.1f lux (0x%06lu @ %.3f)\r\n", (float)al * scale, al, scale);
}

void read_als_color_data(uint8_t addr, absolute_time_t deadline, absolute_time_t start)
{
	if (addr > BUS_ADDR_MAX || is_nil_time(deadline)) return;

	sleep_until(deadline);

	int r = als_check_result(addr, ALS_STATUS_LS_DATA);
	absolute_time_t end = get_absolute_time();
	if (r != PICO_OK)
	{
		printf(" fail(%.3f s|%lld us): %d\r\n",
		       (float)to_ms_since_boot(end)/1000,
		       absolute_time_diff_us(start, end),
		       r);
		return;
	}
	printf("APDS-9999 - done(%.3f s): %lld us\r\n",
	       (float)to_ms_since_boot(end)/1000,
	       absolute_time_diff_us(start, end));

	printf("Reading color data...");
	uint32_t ir, cr, cg, cb;
	r = als_read_ir_rgb(addr, &ir, &cr, &cg, &cb);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n", r);
		return;
	}
	puts(" ok\r");

	printf("APDS-9999 AL - IR: 0x%06lu, R: 0x%06lu, G: 0x%06lu, B: 0x%06lu\r\n",
	       ir, cr, cg, cb);
}
