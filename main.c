#include <stdio.h>
#include <math.h>

#include "pico/binary_info.h"
#include "pico/stdlib.h"

#include "bus.h"
#include "prs.h"
#include "rhs.h"
#include "als.h"

#include "check.h"
#include "meas.h"

#define PROJ_DESC "I2CS Board RPi2 Example"

#define INTERVAL 400
#define TIME_ALIGNMENT 2 // Align measurement cycle to the given value in seconds
#define MEAS_CYCLE 900   // Measurement cycle time estimation in ms

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
	int32_t prs_k, tmp_k;
	setup_prs(prs_addr, &prs_coefs, &prs_k, &tmp_k);

	uint32_t now = to_ms_since_boot(get_absolute_time());
	size_t offset = (size_t)now / 1000 / TIME_ALIGNMENT;
	uint32_t rem = (uint32_t)(offset + 1) * TIME_ALIGNMENT * 1000 - now;
	if (rem < MEAS_CYCLE)
	{
		++offset;
		rem += TIME_ALIGNMENT * 1000;
	}
	printf("Starting at offset %lu (%lu ms remains for the first cycle)\r\n", offset, rem);

	i = 0;
	while(1)
	{
		printf("%lu: %.3f s\r\n", ++i, (float)now / 1000);

		absolute_time_t prs_start = nil_time;
		start_measure_prs_tmp(prs_addr, &prs_start);

		float tmp_raw_sc = NAN;
		read_prs_tmp_raw_data(prs_addr, tmp_k, &tmp_raw_sc, prs_start);

		prs_start = nil_time;
		start_measure_prs(prs_addr, &prs_start);

		absolute_time_t rhs_deadline, rhs_start;
		start_measure_rhs(rhs_addr, &rhs_deadline, &rhs_start);

		uint8_t gain;
		probe_als(als_addr, &gain);

		absolute_time_t als_deadline, als_start;
		start_measure_als(als_addr, false, gain, ALS_MEAS_RES_20,
		                  &als_deadline, &als_start);

		float prs_raw_sc = NAN;
		read_prs_raw_data(prs_addr, prs_k, &prs_raw_sc, prs_start);
		calc_prs(prs_addr, &prs_coefs, tmp_raw_sc, prs_raw_sc);

		read_rhs_data(rhs_addr, rhs_deadline, rhs_start);

		read_als_light_data(als_addr, gain, ALS_MEAS_RES_20,
		                    als_deadline, als_start);

		start_measure_als(als_addr, true, gain, ALS_MEAS_RES_20,
		                  &als_deadline, &als_start);

		read_als_color_data(als_addr, als_deadline, als_start);

		absolute_time_t target = from_us_since_boot((uint64_t)(i + offset) * TIME_ALIGNMENT * 1000000);
		sleep_until(target);

		now = to_ms_since_boot(get_absolute_time());
	}
}
