/***********************************************************************/
/*                                                                     */
/*      PROJECT NAME :  RTOSDemo                                       */
/*      FILE         :  hardware_setup.c                               */
/*      DESCRIPTION  :  Hardware Initialization                        */
/*      CPU SERIES   :  RX600                                          */
/*      CPU TYPE     :  RX64M                                          */
/*                                                                     */
/*      This file is generated by e2 studio.                        */
/*                                                                     */
/***********************************************************************/


/************************************************************************/
/*    File Version: V1.00                                               */
/*    Date Generated: 08/07/2013                                        */
/************************************************************************/

#include "iodefine.h"
#ifdef __cplusplus
extern "C" {
#endif
extern void HardwareSetup( void );
#ifdef __cplusplus
}
#endif


#define _00_CGC_MAINOSC_RESONATOR      ( 0x00U )        /* Resonator */
#define _00_CGC_MAINOSC_UNDER24M       ( 0x00U )        /* 20.1 to 24 MHz */
#define _52_CGC_MOSCWTCR_VALUE         ( 0x52U )        /* Main Clock Oscillator Wait Time */
#define _00000001_CGC_PCLKD_DIV_2      ( 0x00000001UL ) /* x1/2 */
#define _00000010_CGC_PCLKC_DIV_2      ( 0x00000010UL ) /* x1/2 */
#define _00000100_CGC_PCLKB_DIV_2      ( 0x00000100UL ) /* x1/2 */
#define _00001000_CGC_PCLKA_DIV_2      ( 0x00001000UL ) /* x1/2 */
#define _00010000_CGC_BCLK_DIV_2       ( 0x00010000UL ) /* x1/2 */
#define _01000000_CGC_ICLK_DIV_2       ( 0x01000000UL ) /* x1/2 */
#define _10000000_CGC_FCLK_DIV_2       ( 0x10000000UL ) /* x1/2 */
#define _0001_CGC_PLL_FREQ_DIV_2       ( 0x0001U )      /* x1/2 */
#define _1300_CGC_PLL_FREQ_MUL_10_0    ( 0x1300U )      /* x10.0 */
#define _0020_CGC_UCLK_DIV_3           ( 0x0020U )      /* x1/3 */
#define _0400_CGC_CLOCKSOURCE_PLL      ( 0x0400U )      /* PLL circuit */

void R_CGC_Create( void )
{
    /* Set main clock control registers */
    SYSTEM.MOFCR.BYTE = _00_CGC_MAINOSC_RESONATOR | _00_CGC_MAINOSC_UNDER24M;
    SYSTEM.MOSCWTCR.BYTE = _52_CGC_MOSCWTCR_VALUE;

    /* Set main clock operation */
    SYSTEM.MOSCCR.BIT.MOSTP = 0U;

    /* Wait for main clock oscillator wait counter overflow */
    while( 1U != SYSTEM.OSCOVFSR.BIT.MOOVF )
    {
    }

    /* Set system clock */
    SYSTEM.SCKCR.LONG = _00000001_CGC_PCLKD_DIV_2 | _00000010_CGC_PCLKC_DIV_2 | _00000100_CGC_PCLKB_DIV_2 |
                        _00001000_CGC_PCLKA_DIV_2 | _00010000_CGC_BCLK_DIV_2 | _01000000_CGC_ICLK_DIV_2 |
                        _10000000_CGC_FCLK_DIV_2;

    /* Set PLL circuit */
    SYSTEM.PLLCR2.BIT.PLLEN = 0U;
    SYSTEM.PLLCR.BIT.PLLSRCSEL = 0U;
    SYSTEM.PLLCR.WORD = _0001_CGC_PLL_FREQ_DIV_2 | _1300_CGC_PLL_FREQ_MUL_10_0;

    /* Wait for PLL wait counter overflow */
    while( 1U != SYSTEM.OSCOVFSR.BIT.PLOVF )
    {
    }

    /* Disable sub-clock */
    SYSTEM.SOSCCR.BIT.SOSTP = 1U;

    /* Wait for the register modification to complete */
    while( 1U != SYSTEM.SOSCCR.BIT.SOSTP )
    {
    }

    /* Set LOCO */
    SYSTEM.LOCOCR.BIT.LCSTP = 0U;

    /* Set UCLK */
    SYSTEM.SCKCR2.WORD = _0020_CGC_UCLK_DIV_3;

    /* Set SDCLK */
    SYSTEM.SCKCR.BIT.PSTOP0 = 1U;

    /* Set clock source */
    SYSTEM.SCKCR3.WORD = _0400_CGC_CLOCKSOURCE_PLL;
}


void HardwareSetup( void )
{
    /* Enable writing to registers related to operating modes, LPC, CGC and software reset */
    SYSTEM.PRCR.WORD = 0xA50BU;

    /* Enable writing to MPC pin function control registers */
    MPC.PWPR.BIT.B0WI = 0U;
    MPC.PWPR.BIT.PFSWE = 1U;

    /* Set peripheral settings */
    R_CGC_Create();

    /* Disable writing to MPC pin function control registers */
    MPC.PWPR.BIT.PFSWE = 0U;
    MPC.PWPR.BIT.B0WI = 1U;

    /* Enable protection */
    SYSTEM.PRCR.WORD = 0xA500U;
}
