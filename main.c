#include <stdio.h>
#include <math.h>

#include "pico/binary_info.h"
#include "pico/stdlib.h"

#include "bus.h"
#include "prs.h"
#include "rhs.h"
#include "als.h"

#define PROJ_DESC "I2CS Board RPi2 Example"

#define INTERVAL 400
#define PRS_INT_PIN 22

void report_sensor(const char *name, int r, const uint8_t *addr);
void halt_if_no_devices(uint8_t prs_addr, uint8_t rhs_addr, uint8_t als_addr);
void enumerate_sensors(uint8_t *prs_addr, uint8_t *rhs_addr, uint8_t *als_addr);
void check_sensors(uint8_t *prs_addr, uint8_t *rhs_addr, uint8_t *als_addr);
void check_prs_id(uint8_t *addr);
void check_rhs_id(uint8_t *addr);
void check_als_id(uint8_t *addr);

void int_setup(uint gpio);

void read_prs_coefs(uint8_t addr, prs_coefs_t *prs_coefs_t);
void prs_measure(uint8_t addr, size_t n, const prs_coefs_t *coefs);

int main()
{
	bi_decl(bi_program_description(PROJ_DESC));

	stdio_init_all();

	size_t i = 0;
	while (i++ < 10)
	{
		sleep_ms(INTERVAL);
		putchar('.');
	}

	puts("\r\n" PROJ_DESC "\r\n");

	int_setup(PRS_INT_PIN);
	bus_setup();

	uint8_t prs_addr, rhs_addr, als_addr;
	enumerate_sensors(&prs_addr, &rhs_addr, &als_addr);
	check_sensors(&prs_addr, &rhs_addr, &als_addr);

	prs_coefs_t prs_coefs;
	read_prs_coefs(prs_addr, &prs_coefs);

	i = 0;
	while(1)
	{
		prs_measure(prs_addr, ++i, &prs_coefs);
		sleep_ms(INTERVAL);
	}
}

void report_sensor(const char *name, int r, const uint8_t *addr)
{
	if (r < 0)
	{
		printf("\t%s: %s\r\n", name, bus_error_to_str(r));
	}
	else
	{
		printf("\t%s: 0x%02hhx\r\n", name, *addr);
	}
}

void halt_if_no_devices(uint8_t prs_addr, uint8_t rhs_addr, uint8_t als_addr)
{
	if (prs_addr > BUS_ADDR_MAX && rhs_addr > BUS_ADDR_MAX && als_addr > BUS_ADDR_MAX)
	{
		puts("\nNo compatible devices found. Stopping...\r");
		while (1)
		{
			tight_loop_contents();
		}
	}
}

void enumerate_sensors(uint8_t *prs_addr, uint8_t *rhs_addr, uint8_t *als_addr)
{
	puts("Enumerating sensors:\r");
	report_sensor("SPL07-003", prs_find(prs_addr), prs_addr);
	report_sensor("ENS210...", rhs_find(rhs_addr), rhs_addr);
	report_sensor("APDS-9999", als_find(als_addr), als_addr);

	halt_if_no_devices(*prs_addr, *rhs_addr, *als_addr);
}

void check_sensors(uint8_t *prs_addr, uint8_t *rhs_addr, uint8_t *als_addr)
{
	puts("\nChecking sensor IDs:\r");
	if (*prs_addr <= BUS_ADDR_MAX) check_prs_id(prs_addr);
	if (*rhs_addr <= BUS_ADDR_MAX) check_rhs_id(rhs_addr);
	if (*als_addr <= BUS_ADDR_MAX) check_als_id(als_addr);

	halt_if_no_devices(*prs_addr, *rhs_addr, *als_addr);
}

void check_prs_id(uint8_t *addr)
{
	uint8_t id;
	int r = prs_get_id(*addr, &id);
	if (r != PICO_OK)
	{
		*addr = BUS_ADDR_INVALID;
		printf("\tSPL07-003 ID: %s. Ignoring device...\r\n", bus_error_to_str(r));
	}
	else if (id != PRS_ID)
	{
		*addr = BUS_ADDR_INVALID;
		printf("\tSPL07-003 ID: %hhu.%hhu (invalid, expected %hhu.%hhu). Ignoring device...\r\n",
		       id & 0x0f, (id & 0xf0) >> 4,
		       PRS_ID & 0x0f, (PRS_ID & 0xf0) >> 4);
	}
	else
	{
		printf("\tSPL07-003 ID: %hhu.%hhu\r\n", id & 0x0f, (id & 0xf0) >> 4);
	}
}

void check_rhs_id(uint8_t *addr)
{
	uint16_t id, rev;
	uint64_t uid;

	int r = rhs_get_id(*addr, &id, &rev, &uid);
	if (r != PICO_OK)
	{
		*addr = BUS_ADDR_INVALID;
		printf("\tENS210 ID...: %s. Ignoring device...\r\n", bus_error_to_str(r));
	}
	else if (id != RHS_ID || rev != RHS_REV)
	{
		*addr = BUS_ADDR_INVALID;
		printf("\tENS210 ID...: %hx.%hu (invalid, expected %hx.%hu). Ignoring device...\r\n",
		       id, rev, RHS_ID, RHS_REV);
	}
	else
	{
		printf("\tENS210 ID...: %hx.%hu (%016llX)\r\n", id, rev, uid);
	}
}

void check_als_id(uint8_t *addr)
{
	uint8_t id;
	int r = als_get_id(*addr, &id);
	if (r != PICO_OK)
	{
		*addr = BUS_ADDR_INVALID;
		printf("\tAPDS-9999 ID: %s. Ignoring device...\r\n", bus_error_to_str(r));
	}
	else if (id != ALS_ID)
	{
		*addr = BUS_ADDR_INVALID;
		printf("\tAPDS-9999 ID: %hhu.%hhu (invalid, expected %hhu.%hhu). Ignoring device...\r\n",
		       (id & 0xf0) >> 4, id & 0x0f,
		       (ALS_ID & 0xf0) >> 4, ALS_ID & 0x0f);
	}
	else
	{
		printf("\tAPDS-9999 ID: %hhu.%hhu\r\n",
		       (id & 0xf0) >> 4, id & 0x0f);
	}
}

void read_prs_coefs(uint8_t addr, prs_coefs_t *coefs)
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
}

void int_setup(uint gpio)
{
	gpio_init(gpio);
	gpio_set_dir(gpio, GPIO_IN);
	gpio_pull_up(gpio);
}

void prs_clr_int(uint8_t addr)
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

int prs_wait_for_value(uint8_t addr, int32_t prs_k, int32_t tmp_k, float *p_raw_sc, float *t_raw_sc)
{
	absolute_time_t start = get_absolute_time();
	for (int i=1; i < 100000000; i++)
	{
		if (!gpio_get(PRS_INT_PIN))
		{
			absolute_time_t end = get_absolute_time();
			printf(" done (int@%d - 0): %lld us (%d clk)\r\n",
			       PRS_INT_PIN, absolute_time_diff_us(start, end), i);

			printf("Reading pressure sensor operating status...");
			uint8_t cfg = 0;
			int r = bus_read_byte(addr, PRS_MEAS_CFG, &cfg);
			if (r != PICO_OK)
			{
				printf(" error: %d\r\n", r);
				return r;
			}
			printf(" PRS_RDY: %c, TMP_RDY: %c\r\n",
			       (cfg & PRS_PRS_RDY)? 'Y':'N',
			       (cfg & PRS_TMP_RDY)? 'Y':'N');

			prs_clr_int(addr);

			if (cfg & PRS_PRS_RDY)
			{
				printf("Reading raw pressure value...");
				int32_t raw;
				r = prs_get_prs_raw(addr, &raw);
				if (r != PICO_OK)
				{
					printf(" error: %d\r\n");
				}
				else
				{
					*p_raw_sc = (float)raw/prs_k;
					printf(" p_raw: %d, p_raw_sc: %.7f\r\n", raw, *p_raw_sc);
				}
			}

			if (cfg & PRS_TMP_RDY)
			{
				printf("Reading raw temperature value from pressure sensor...");
				int32_t raw;
				r = prs_get_tmp_raw(addr, &raw);
				if (r != PICO_OK)
				{
					printf(" error: %d\r\n");
				}
				else
				{
					*t_raw_sc = (float)raw/tmp_k;
					printf(" t_raw: %d, t_raw_sc: %.7f\r\n", raw, *t_raw_sc);
				}
			}

			return PICO_OK;
		}

		if (!(i%40000))
		{
			puts(".\r");
		}
		else if (!(i%1000))
		{
			putchar('.');
		}
	}

	return PICO_ERROR_GENERIC;
}

void prs_measure(uint8_t addr, size_t n, const prs_coefs_t *coefs)
{
	if (addr > BUS_ADDR_MAX) return;

	int32_t prs_k = prs_prc_to_k(PRS_PRC_16);
	int32_t tmp_k = prs_prc_to_k(PRS_PRC_1);
	printf("\nConfiguring pressure sensor (p scale %d, t scale %d)...", prs_k, tmp_k);
	int r = prs_config(addr, PRS_PRC_16, PRS_PRC_1);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n", r);
		return;
	}
	puts(" ok\r");

	printf("Interrupt pin(%d): %d\r\n", PRS_INT_PIN, gpio_get(PRS_INT_PIN));

	printf("Starting pressure measurement...");
	r = prs_meas_config(addr);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n", r);
		return;
	}
	puts(" ok\r");

	float p_raw_sc = NAN, t_raw_sc = NAN;
	if (prs_wait_for_value(addr, prs_k, tmp_k, &p_raw_sc, &t_raw_sc) != PICO_OK)
	{
		return;
	}

	if ((isnan(p_raw_sc) || isnan(t_raw_sc)) &&
	    prs_wait_for_value(addr, prs_k, tmp_k, &p_raw_sc, &t_raw_sc) != PICO_OK)
	{
		return;
	}

	printf("Stopping measurement... ");
	r = prs_meas_stop(addr);
	if (r != PICO_OK)
	{
		printf(" error: %d\r\n");
	}
	else
	{
		puts(" ok\r");
	}

	if (isnan(t_raw_sc))
	{
		puts("SPL07-003 - T: NaN, P: NaN\r");
		return;
	}

	float t = coefs->c0*0.5 + coefs->c1 * t_raw_sc;

	if (isnan(p_raw_sc))
	{
		printf("SPL07-003 - T: %.2f C, P: NaN\r\n", t);
		return;
	}

	float p = p_raw_sc;
	float p2 = p * p;
	float p3 = p2 * p;
	float p4 = p3 * p;
	float pt_corr = t_raw_sc*(coefs->c01 + coefs->c11*p + coefs->c21*p2 + coefs->c31*p3);

	p = pt_corr + coefs->c00 + coefs->c10*p + coefs->c20*p2 + coefs->c30*p3 + coefs->c40*p4;
	printf("SPL07-003(%09lu) - T: %.1f C, P: %.2f mbar\r\n", n, t, p/100);
}
