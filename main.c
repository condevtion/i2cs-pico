#include <stdio.h>

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

	i = 0;
	while(1)
	{
		i++;

		start_measure_prs(prs_addr);
		if (i > 1)
		{
			absolute_time_t rhs_deadline, rhs_start;
			start_measure_rhs(rhs_addr, &rhs_deadline, &rhs_start);

			read_rhs_data(rhs_addr, rhs_deadline, rhs_start);
		}
		read_prs_data(prs_addr, i, &prs_coefs, prs_k, tmp_k);
	}
}
