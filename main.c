#include <stdio.h>

#include "pico/binary_info.h"
#include "pico/stdlib.h"

#define PROJ_DESC "I2CS Board RPi2 Example"

#define INTERVAL 400

int main()
{
	bi_decl(bi_program_description(PROJ_DESC));
	stdio_init_all();

	for (int i = 0; i < 10; i++)
	{
		sleep_ms(INTERVAL);
		putchar('.');
	}

	puts("\r\n" PROJ_DESC "\r\n");

	while (1)
	{
		tight_loop_contents();
	}
}
