/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
 * !!GlobalInfo
 * product: Pins v10.0
 * processor: LPC55S69
 * package_id: LPC55S69JBD100
 * mcu_data: ksdk2_0
 * processor_version: 10.0.0
 * pin_labels:
 * - {pin_num: '1', pin_signal: PIO1_4/FC0_SCK/SD0_D0/CTIMER2_MAT1/SCT0_OUT0/FREQME_GPIO_CLK_A, label: LED_BLUE, identifier: LED_BLUE}
 * - {pin_num: '5', pin_signal: PIO1_6/FC0_TXD_SCL_MISO_WS/SD0_D3/CTIMER2_MAT1/SCT_GPI3, label: LED_RED, identifier: LED_RED}
 * - {pin_num: '9', pin_signal: PIO1_7/FC0_RTS_SCL_SSEL1/SD0_D1/CTIMER2_MAT2/SCT_GPI4, label: LED_GREEN, identifier: LED_GREEN}
 * - {pin_num: '10', pin_signal: PIO1_9/FC1_SCK/CT_INP4/SCT0_OUT2/FC4_CTS_SDA_SSEL0/ADC0_12, label: USER_BUTTON, identifier: USER_BUTTON}
 * - {pin_num: '94', pin_signal: PIO0_30/FC0_TXD_SCL_MISO_WS/SD1_D3/CTIMER0_MAT0/SCT0_OUT9/SECURE_GPIO0_30, label: UART_TX, identifier: UART_TX}
 * - {pin_num: '92', pin_signal: PIO0_29/FC0_RXD_SDA_MOSI_DATA/SD1_D2/CTIMER2_MAT3/SCT0_OUT8/CMP0_OUT/PLU_OUT2/SECURE_GPIO0_29, label: UART_RX, identifier: UART_RX}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_iocon.h"
#include "pin_mux.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins( void )
{
    BOARD_InitPins();
}

/* clang-format off */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
 * BOARD_InitPins:
 * - options: {callFromInitBoot: 'true', coreID: cm33_core0, enableClock: 'true'}
 * - pin_list:
 * - {pin_num: '92', peripheral: FLEXCOMM0, signal: RXD_SDA_MOSI_DATA, pin_signal: PIO0_29/FC0_RXD_SDA_MOSI_DATA/SD1_D2/CTIMER2_MAT3/SCT0_OUT8/CMP0_OUT/PLU_OUT2/SECURE_GPIO0_29,
 *  direction: INPUT, mode: inactive, slew_rate: standard, invert: disabled, open_drain: disabled}
 * - {pin_num: '94', peripheral: FLEXCOMM0, signal: TXD_SCL_MISO_WS, pin_signal: PIO0_30/FC0_TXD_SCL_MISO_WS/SD1_D3/CTIMER0_MAT0/SCT0_OUT9/SECURE_GPIO0_30, direction: OUTPUT,
 *  mode: inactive, slew_rate: standard, invert: disabled, open_drain: disabled}
 * - {pin_num: '1', peripheral: GPIO, signal: 'PIO1, 4', pin_signal: PIO1_4/FC0_SCK/SD0_D0/CTIMER2_MAT1/SCT0_OUT0/FREQME_GPIO_CLK_A, direction: OUTPUT, gpio_init_state: 'true'}
 * - {pin_num: '5', peripheral: GPIO, signal: 'PIO1, 6', pin_signal: PIO1_6/FC0_TXD_SCL_MISO_WS/SD0_D3/CTIMER2_MAT1/SCT_GPI3, direction: OUTPUT, gpio_init_state: 'true'}
 * - {pin_num: '9', peripheral: GPIO, signal: 'PIO1, 7', pin_signal: PIO1_7/FC0_RTS_SCL_SSEL1/SD0_D1/CTIMER2_MAT2/SCT_GPI4, direction: OUTPUT, gpio_init_state: 'true'}
 * - {pin_num: '10', peripheral: GPIO, signal: 'PIO1, 9', pin_signal: PIO1_9/FC1_SCK/CT_INP4/SCT0_OUT2/FC4_CTS_SDA_SSEL0/ADC0_12}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
/* Function assigned for the Cortex-M33 (Core #0) */
void BOARD_InitPins( void )
{
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock( kCLOCK_Iocon );

    /* Enables the clock for the GPIO1 module */
    CLOCK_EnableClock( kCLOCK_Gpio1 );

    gpio_pin_config_t LED_BLUE_config =
    {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 1U
    };
    /* Initialize GPIO functionality on pin PIO1_4 (pin 1)  */
    GPIO_PinInit( BOARD_INITPINS_LED_BLUE_GPIO, BOARD_INITPINS_LED_BLUE_PORT, BOARD_INITPINS_LED_BLUE_PIN, &LED_BLUE_config );

    gpio_pin_config_t LED_RED_config =
    {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 1U
    };
    /* Initialize GPIO functionality on pin PIO1_6 (pin 5)  */
    GPIO_PinInit( BOARD_INITPINS_LED_RED_GPIO, BOARD_INITPINS_LED_RED_PORT, BOARD_INITPINS_LED_RED_PIN, &LED_RED_config );

    gpio_pin_config_t LED_GREEN_config =
    {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 1U
    };
    /* Initialize GPIO functionality on pin PIO1_7 (pin 9)  */
    GPIO_PinInit( BOARD_INITPINS_LED_GREEN_GPIO, BOARD_INITPINS_LED_GREEN_PORT, BOARD_INITPINS_LED_GREEN_PIN, &LED_GREEN_config );

    const uint32_t UART_RX = ( /* Pin is configured as FC0_RXD_SDA_MOSI_DATA */
        IOCON_PIO_FUNC1 |
        /* No addition pin function */
        IOCON_PIO_MODE_INACT |
        /* Standard mode, output slew rate control is enabled */
        IOCON_PIO_SLEW_STANDARD |
        /* Input function is not inverted */
        IOCON_PIO_INV_DI |
        /* Enables digital function */
        IOCON_PIO_DIGITAL_EN |
        /* Open drain is disabled */
        IOCON_PIO_OPENDRAIN_DI );
    /* PORT0 PIN29 (coords: 92) is configured as FC0_RXD_SDA_MOSI_DATA */
    IOCON_PinMuxSet( IOCON, BOARD_INITPINS_UART_RX_PORT, BOARD_INITPINS_UART_RX_PIN, UART_RX );

    const uint32_t UART_TX = ( /* Pin is configured as FC0_TXD_SCL_MISO_WS */
        IOCON_PIO_FUNC1 |
        /* No addition pin function */
        IOCON_PIO_MODE_INACT |
        /* Standard mode, output slew rate control is enabled */
        IOCON_PIO_SLEW_STANDARD |
        /* Input function is not inverted */
        IOCON_PIO_INV_DI |
        /* Enables digital function */
        IOCON_PIO_DIGITAL_EN |
        /* Open drain is disabled */
        IOCON_PIO_OPENDRAIN_DI );
    /* PORT0 PIN30 (coords: 94) is configured as FC0_TXD_SCL_MISO_WS */
    IOCON_PinMuxSet( IOCON, BOARD_INITPINS_UART_TX_PORT, BOARD_INITPINS_UART_TX_PIN, UART_TX );

    IOCON->PIO[ 1 ][ 4 ] = ( ( IOCON->PIO[ 1 ][ 4 ] &
                               /* Mask bits to zero which are setting */
                               ( ~( IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK ) ) )

                             /* Selects pin function.
                              * : PORT14 (pin 1) is configured as PIO1_4. */
                             | IOCON_PIO_FUNC( PIO1_4_FUNC_ALT0 )

                             /* Select Digital mode.
                              * : Enable Digital mode.
                              * Digital input is enabled. */
                             | IOCON_PIO_DIGIMODE( PIO1_4_DIGIMODE_DIGITAL ) );

    IOCON->PIO[ 1 ][ 6 ] = ( ( IOCON->PIO[ 1 ][ 6 ] &
                               /* Mask bits to zero which are setting */
                               ( ~( IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK ) ) )

                             /* Selects pin function.
                              * : PORT16 (pin 5) is configured as PIO1_6. */
                             | IOCON_PIO_FUNC( PIO1_6_FUNC_ALT0 )

                             /* Select Digital mode.
                              * : Enable Digital mode.
                              * Digital input is enabled. */
                             | IOCON_PIO_DIGIMODE( PIO1_6_DIGIMODE_DIGITAL ) );

    IOCON->PIO[ 1 ][ 7 ] = ( ( IOCON->PIO[ 1 ][ 7 ] &
                               /* Mask bits to zero which are setting */
                               ( ~( IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK ) ) )

                             /* Selects pin function.
                              * : PORT17 (pin 9) is configured as PIO1_7. */
                             | IOCON_PIO_FUNC( PIO1_7_FUNC_ALT0 )

                             /* Select Digital mode.
                              * : Enable Digital mode.
                              * Digital input is enabled. */
                             | IOCON_PIO_DIGIMODE( PIO1_7_DIGIMODE_DIGITAL ) );

    if( Chip_GetVersion() == 1 )
    {
        IOCON->PIO[ 1 ][ 9 ] = ( ( IOCON->PIO[ 1 ][ 9 ] &
                                   /* Mask bits to zero which are setting */
                                   ( ~( IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK ) ) )

                                 /* Selects pin function.
                                  * : PORT19 (pin 10) is configured as PIO1_9. */
                                 | IOCON_PIO_FUNC( PIO1_9_FUNC_ALT0 )

                                 /* Select Digital mode.
                                  * : Enable Digital mode.
                                  * Digital input is enabled. */
                                 | IOCON_PIO_DIGIMODE( PIO1_9_DIGIMODE_DIGITAL ) );
    }
    else
    {
        IOCON->PIO[ 1 ][ 9 ] = ( ( IOCON->PIO[ 1 ][ 9 ] &
                                   /* Mask bits to zero which are setting */
                                   ( ~( IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK ) ) )

                                 /* Selects pin function.
                                  * : PORT19 (pin 10) is configured as PIO1_9. */
                                 | IOCON_PIO_FUNC( PIO1_9_FUNC_ALT0 )

                                 /* Select Digital mode.
                                  * : Enable Digital mode.
                                  * Digital input is enabled. */
                                 | IOCON_PIO_DIGIMODE( PIO1_9_DIGIMODE_DIGITAL ) );
    }
}

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
