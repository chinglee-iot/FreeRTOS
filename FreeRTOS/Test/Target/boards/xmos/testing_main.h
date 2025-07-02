// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef TESTING_MAIN_H_
#define TESTING_MAIN_H_

#define testingmainNUM_TILES				2

/* TODO: Update to be package specific check */
#if ( testingmainNUM_TILES > 2 )
#error Tiles must be less than 3.
#endif

/* The constants used in the idle task calculation. */
#define intCONST1				( ( BaseType_t ) 346 )
#define intCONST2				( ( BaseType_t ) 74324 )
#define intCONST3				( ( BaseType_t ) -2 )
#define intCONST4				( ( BaseType_t ) 9 )
#define intEXPECTED_ANSWER		( ( BaseType_t ) ( ( intCONST1 + intCONST2 ) * intCONST3 ) / intCONST4 )

#endif /* TESTING_MAIN_H_ */
