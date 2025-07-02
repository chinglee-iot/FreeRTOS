// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>

#include "rtos_printf.h"
#include "testing_main.h"

extern "C" {
int c_main( void );
}

void xctask1(void)
{
	rtos_printf("xc task 1 on core %u\n", get_logical_core_id());
	for (;;);
}

void xctask2(void)
{
	rtos_printf("xc task 2 on core %u\n", get_logical_core_id());
	for (;;);
}

int main(void)
{
	par {
		on tile[0]:
		{
			par {
				c_main();
			}
		}
#if ( testingmainNUM_TILES > 1 )
		on tile[1]:
		{
			par {
				/* We only use tile 0 to run SMP on target test. */
				xctask2();
			}
		}
#endif
	}
	return 0;
}
