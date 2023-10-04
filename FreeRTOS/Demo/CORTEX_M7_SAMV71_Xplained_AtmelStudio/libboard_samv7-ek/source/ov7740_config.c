/* ----------------------------------------------------------------------------
 *         SAM Software Package License
 * ----------------------------------------------------------------------------
 * Copyright (c) 2013, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/**
 * \file
 */

#include "include/ov_7740.h"
#include "board.h"


#ifdef __cplusplus
extern "C" {
#endif

#define FRAMERATE    30

/**
 * \defgroup ov7740_CMOS_image_sensor_registers_group
 *
 * This file defines several arrays. Each of them contain address of ov7740
 * register and corresponding value for a specific configuration.
 * There are seven different configuration:
 *       -  OV7740_VGA_YUV422[]
 *       -  OV7740_QVGA_YUV422[]
 *       -  OV7740_QQVGA_YUV422[]
 *       -  OV7740_QVGA_RGB888[]
 *       -  OV7740_QQVGA_RGB888[]
 *       -  OV7740_TEST_PATTERN[]
 *
 * These values have been obtained from OV7740 datasheet and OmniVision
 * Developer Kit.
 * @{
 */

/*------------------------------------------------------------------------------
 *         Local Variables
 *----------------------------------------------------------------------------*/

/**
 * \brief Addresses and values of the OV7740 registers for the
 * OV7740_VGA_YUV422 configuration:
 *  - 640*4800 pixel by picture (VGA)
 *  - pixel data in YUV422 format (Y1, U, Y2, V)
 */
const struct ov_reg OV7740_VGA_YUV422[] =
{
    { 0x12,       0x80            },
    /* flag for soft reset delay */
    { 0x55,       PLL_DIV_DEFAULT },

    /**************************************************************/
    /*  30fps  11 01 ;clock_divider ;sysclk=24MHz at XCLK=24MHz   */
    /*  20fps  11 02 ;clock_divider ;sysclk=16MHz at XCLK=24MHz   */
    /*  15fps  11 03 ;clock_divider ;sysclk=12MHz at XCLK=24MHz   */
    /*  10fps  11 05 ;sysclk=8MHz at XCLK=24MHz                   */
    /*  7p5fps 11 07 ;sysclk=6MHz at XCLK=24MHz  ((PLL/2)/16) (PLL=792)*/
    /**************************************************************/
    /*  7p5fps 11 0x0A ;sysclk=6MHz at XCLK=16.5MHz(MCK_132)/8  */
    /**************************************************************/

    { OV7740_CLK, FRAME_RATE_30   },
    /**************************************************************/

    { 0x12,       0x00            },
    { 0xd5,       0x10            },
    { 0x0c,       0x12            },
    { 0x0d,       0x34            },
    { 0x17,       0x25            },
    { 0x18,       0xa0            },
    { 0x19,       0x03            },
    { 0x1a,       0xf0            },
    { 0x1b,       0x89            },
    { 0x22,       0x03            },
    { 0x29,       0x18            },
    { 0x2b,       0xf8            },
    { 0x2c,       0x01            },
    { 0x31,       0xa0            },
    { 0x32,       0xf0            },
    { 0x33,       0xc4            },
    { 0x35,       0x05            },
    { 0x36,       0x3f            },

    { 0x04,       0x60            },
    { 0x27,       0x80            },
    { 0x3d,       0x0f            },
    { 0x3e,       0x80            },
    { 0x3f,       0x40            },
    { 0x40,       0x7f            },
    { 0x41,       0x6a            },
    { 0x42,       0x29            },
    { 0x44,       0x22            },
    { 0x45,       0x41            },
    { 0x47,       0x02            },
    { 0x49,       0x64            },
    { 0x4a,       0xa1            },
    { 0x4b,       0x40            },
    { 0x4c,       0x1a            },
    { 0x4d,       0x50            },
    { 0x4e,       0x13            },
    { 0x64,       0x00            },
    { 0x67,       0x88            },
    { 0x68,       0x1a            },

    { 0x14,       0x28            }, /*;38/28/18 for 16/8/4x gain ceiling */
    { 0x24,       0x3c            },
    { 0x25,       0x30            },
    { 0x26,       0x72            },
    { 0x50,       0x97            },
    { 0x51,       0x7e            },
    { 0x52,       0x00            },
    { 0x53,       0x00            },
    { 0x20,       0x00            },
    { 0x21,       0x23            },
    { 0x50,       0x97            }, /* ;12e/97/4b/25 for 60/30/15/7.5fps, 50Hz */

    /*********************************/
    /* Normal Mode / No test pattern */
    { 0x38,       0x14            },
    /*********************************/
    { 0xe9,       0x00            },
    { 0x56,       0x55            },
    { 0x57,       0xff            },
    { 0x58,       0xff            },
    { 0x59,       0xff            },
    { 0x5f,       0x04            },
    { 0xec,       0x00            },
    { 0x13,       0xff            },

    { 0x80,       0x7f            },
    { 0x81,       0x3f            },
    { 0x82,       0x32            },
    { 0x83,       0x01            },
    { 0x38,       0x11            },
    { 0x84,       0x70            },
    { 0x85,       0x00            },
    { 0x86,       0x03            },
    { 0x87,       0x01            },
    { 0x88,       0x05            },
    { 0x89,       0x30            },
    { 0x8d,       0x30            },
    { 0x8f,       0x85            },
    { 0x93,       0x30            },
    { 0x95,       0x85            },
    { 0x99,       0x30            },
    { 0x9b,       0x85            },

    { 0x9c,       0x08            },
    { 0x9d,       0x12            },
    { 0x9e,       0x23            },
    { 0x9f,       0x45            },
    { 0xa0,       0x55            },
    { 0xa1,       0x64            },
    { 0xa2,       0x72            },
    { 0xa3,       0x7f            },
    { 0xa4,       0x8b            },
    { 0xa5,       0x95            },
    { 0xa6,       0xa7            },
    { 0xa7,       0xb5            },
    { 0xa8,       0xcb            },
    { 0xa9,       0xdd            },
    { 0xaa,       0xec            },
    { 0xab,       0x1a            },

    { 0xce,       0x78            },
    { 0xcf,       0x6e            },
    { 0xd0,       0x0a            },
    { 0xd1,       0x0c            },
    { 0xd2,       0x84            },
    { 0xd3,       0x90            },
    { 0xd4,       0x1e            },

    { 0x5a,       0x24            },
    { 0x5b,       0x1f            },
    { 0x5c,       0x88            },
    { 0x5d,       0x60            },

    { 0xac,       0x6e            },
    { 0xbe,       0xff            },
    { 0xbf,       0x00            },

    { 0x0f,       0x1d            },
    { 0x0f,       0x1f            },
    { 0xFF,       0xFF            }
};

/**
 * \brief Addresses and values of the OV7740 registers for the
 * OV7740_QVGA_YUV422 configuration:
 *  - 320*240 pixel by picture (QVGA)
 *  - pixel data in YUV422 format (Y1, U, Y2, V)
 *
 */
const struct ov_reg OV7740_QVGA_YUV422[] =
{
    { 0x12,       0x80              },
    /* flag for soft reset delay */
    { 0x55,       PLL_DIV_7_MCK_132 },

    /**************************************************************/
    /*  30fps  11 01 ;clock_divider ;sysclk=24MHz at XCLK=24MHz   */
    /*  20fps  11 02 ;clock_divider ;sysclk=16MHz at XCLK=24MHz   */
    /*  15fps  11 03 ;clock_divider ;sysclk=12MHz at XCLK=24MHz   */
    /*  10fps  11 05 ;sysclk=8MHz at XCLK=24MHz                   */
    /*  7p5fps 11 07 ;sysclk=6MHz at XCLK=24MHz                   */
    /**************************************************************/
    /*  7p5fps 11 0x0A ;sysclk=6MHz at XCLK=16.5MHz  (MCK=132/8)  */
    /**************************************************************/
    { OV7740_CLK, FRAME_RATE_30     },
    /**************************************************************/

    { 0x12,       0x00              },
    { 0xd5,       0x10              },
    { 0x0c,       0x12              },
    { 0x0d,       0x34              },
    { 0x17,       0x25              },
    { 0x18,       0xa0              },
    { 0x19,       0x03              },
    { 0x1a,       0xf0              },
    { 0x1b,       0x89              }, /*;was 81 */
    { 0x22,       0x03              }, /*;new */
    { 0x29,       0x18              }, /*;was 17 */
    { 0x2b,       0xf8              },
    { 0x2c,       0x01              },
    { 0x31,       0xa0              },
    { 0x32,       0xf0              },
    { 0x33,       0xc4              }, /*;was44 */
    { 0x35,       0x05              }, /*;new */
    { 0x36,       0x3f              },

    { 0x04,       0x60              },
    { 0x27,       0x80              }, /*;delete "42 3a b4" */
    { 0x3d,       0x0f              },
    { 0x3e,       0x80              },
    { 0x3f,       0x40              },
    { 0x40,       0x7f              },
    { 0x41,       0x6a              },
    { 0x42,       0x29              },
    { 0x44,       0x22              }, /*;was 11 */
    { 0x45,       0x41              },
    { 0x47,       0x02              },
    { 0x49,       0x64              },
    { 0x4a,       0xa1              },
    { 0x4b,       0x40              },
    { 0x4c,       0x1a              },
    { 0x4d,       0x50              },
    { 0x4e,       0x13              },
    { 0x64,       0x00              },
    { 0x67,       0x88              },
    { 0x68,       0x1a              },

    { 0x14,       0x28              }, /*;38/28/18 for 16/8/4x gain ceiling */
    { 0x24,       0x3c              },
    { 0x25,       0x30              },
    { 0x26,       0x72              },
    { 0x50,       0x97              },
    { 0x51,       0x7e              },
    { 0x52,       0x00              },
    { 0x53,       0x00              },
    { 0x20,       0x00              },
    { 0x21,       0x23              },
    /*********************************/
    /* To enable Static Test Pattern */
    /*********************************/
    /* {0x38, 0x07}, */
    /* {0x84, 0x02}, */

    /*********************************/
    /* Normal Mode / No test pattern */
    { 0x38,       0x14              },
    /*********************************/
    { 0xe9,       0x00              },
    { 0x56,       0x55              },
    { 0x57,       0xff              },
    { 0x58,       0xff              },
    { 0x59,       0xff              },
    { 0x5f,       0x04              },
    { 0xec,       0x00              },
    { 0x13,       0xff              },

    { 0x80,       0x7f              },
    { 0x81,       0x3f              },
    { 0x82,       0x32              },
    { 0x83,       0x01              },
    { 0x38,       0x11              },
    { 0x84,       0x70              },
    { 0x85,       0x00              },
    { 0x86,       0x03              },
    { 0x87,       0x01              },
    { 0x88,       0x05              },
    { 0x89,       0x30              },
    { 0x8d,       0x30              },
    { 0x8f,       0x85              },
    { 0x93,       0x30              },
    { 0x95,       0x85              },
    { 0x99,       0x30              },
    { 0x9b,       0x85              },

    { 0x9c,       0x08              },
    { 0x9d,       0x12              },
    { 0x9e,       0x23              },
    { 0x9f,       0x45              },
    { 0xa0,       0x55              },
    { 0xa1,       0x64              },
    { 0xa2,       0x72              },
    { 0xa3,       0x7f              },
    { 0xa4,       0x8b              },
    { 0xa5,       0x95              },
    { 0xa6,       0xa7              },
    { 0xa7,       0xb5              },
    { 0xa8,       0xcb              },
    { 0xa9,       0xdd              },
    { 0xaa,       0xec              },
    { 0xab,       0x1a              },

    { 0xce,       0x78              },
    { 0xcf,       0x6e              },
    { 0xd0,       0x0a              },
    { 0xd1,       0x0c              },
    { 0xd2,       0x84              },
    { 0xd3,       0x90              },
    { 0xd4,       0x1e              },

    { 0x5a,       0x24              },
    { 0x5b,       0x1f              },
    { 0x5c,       0x88              },
    { 0x5d,       0x60              },

    { 0xac,       0x6e              },
    { 0xbe,       0xff              },
    { 0xbf,       0x00              },

    /* 320x240 */

    { 0x31,       0x50              },
    { 0x32,       0x78              },
    { 0x82,       0x3f              },

    { 0x0f,       0x1d              },
    { 0x0f,       0x1f              },
    { 0xFF,       0xFF              }
};


/**
 * \brief Addresses and values of the OV7740 registers for the
 * OV7740_QQVGA_YUV422 configuration:
 *  - 160*120 pixel by picture (QQVGA)
 *  - pixel data in YUV422 format (Y1, U, Y2, V)
 */
const struct ov_reg OV7740_QQVGA_YUV422[] =
{
    { OV7740_REG0E,      OV7740_REG0E_OUTPUT_1X         },
    { OV7740_REG12,      OV7740_REG12_RESET             },
    { 0x13,              0x00                           },
    { 0x55,              0x40                           },

    /**************************************************************/
    /*  30fps  11 01 ;clock_divider ;sysclk=24MHz at XCLK=24MHz   */
    /*  20fps  11 02 ;clock_divider ;sysclk=16MHz at XCLK=24MHz   */
    /*  15fps  11 03 ;clock_divider ;sysclk=12MHz at XCLK=24MHz   */
    /*  10fps  11 05 ;sysclk=8MHz at XCLK=24MHz                   */
    /*  7p5fps 11 07 ;sysclk=6MHz at XCLK=24MHz                   */
    /**************************************************************/
    /*  7p5fps 11 0x0A ;sysclk=6MHz at XCLK=16.5MHz  (MCK=132/8)  */
    /**************************************************************/

    { OV7740_CLK,        FRAME_RATE_30                  },

    { 0x12,              0x00                           },
    { 0xd5,              0x10                           },
    { OV7740_REG0C,      OV7740_REG0C_MAX_EXPOSURE( 2 ) },
    { 0x0d,              0x34                           },
    { 0x16,              0x01                           },
    { 0x17,              0x25                           },
    { 0x18,              0xa0                           },
    { 0x19,              0x03                           },
    { 0x1a,              0xf0                           },
    { 0x1b,              0x89                           },
    { 0x22,              0x03                           },
    { 0x29,              0x18                           },
    { 0x2b,              0xf8                           },
    { 0x2c,              0x01                           },
    { 0x31,              0xa0                           },
    { 0x32,              0xf0                           },
    { 0x33,              0xc4                           },
    { 0x3a,              0xb4                           },
    { 0x36,              0x3f                           },

    { 0x04,              0x60                           },
    { 0x27,              0x80                           },
    { 0x3d,              0x0f                           },
    { 0x3e,              0x80                           },
    { 0x3f,              0x40                           },
    { 0x40,              0x7f                           },
    { 0x41,              0x6a                           },
    { 0x42,              0x29                           },
    { 0x44,              0xe5                           },
    { 0x45,              0x41                           },
    { 0x47,              0x02                           },
    { 0x49,              0x64                           },
    { 0x4a,              0xa1                           },
    { 0x4b,              0x70                           },
    { 0x4c,              0x1a                           },
    { 0x4d,              0x50                           },
    { 0x4e,              0x13                           },
    { 0x64,              0x00                           },
    { 0x67,              0x88                           },
    { 0x68,              0x1a                           },

    { 0x14,              0x38                           },
    { 0x24,              0x3c                           },
    { 0x25,              0x30                           },
    { 0x26,              0x72                           },
    { 0x50,              0x97                           },
    { 0x51,              0x7e                           },
    { 0x52,              0x00                           },
    { 0x53,              0x00                           },
    { 0x20,              0x00                           },
    { 0x21,              0x23                           },
    { 0x38,              0x14                           },
    { 0xe9,              0x00                           },
    { 0x56,              0x55                           },
    { 0x57,              0xff                           },
    { 0x58,              0xff                           },
    { 0x59,              0xff                           },
    { 0x5f,              0x04                           },
    { 0xec,              0x00                           },
    { 0x13,              0xff                           },

    { 0x80,              0x7f                           },
    { 0x81,              0x3f                           },
    { 0x82,              0x32                           },
    { 0x83,              0x01                           },
    { 0x38,              0x11                           },
    { 0x84,              0x70                           },
    { 0x85,              0x00                           },
    { 0x86,              0x03                           },
    { 0x87,              0x01                           },
    { 0x88,              0x05                           },
    { 0x89,              0x30                           },
    { 0x8d,              0x30                           },
    { 0x8f,              0x85                           },
    { 0x93,              0x30                           },
    { 0x95,              0x85                           },
    { 0x99,              0x30                           },
    { 0x9b,              0x85                           },

    { 0x9c,              0x08                           },
    { 0x9d,              0x12                           },
    { 0x9e,              0x23                           },
    { 0x9f,              0x45                           },
    { 0xa0,              0x55                           },
    { 0xa1,              0x64                           },
    { 0xa2,              0x72                           },
    { 0xa3,              0x7f                           },
    { 0xa4,              0x8b                           },
    { 0xa5,              0x95                           },
    { 0xa6,              0xa7                           },
    { 0xa7,              0xb5                           },
    { 0xa8,              0xcb                           },
    { 0xa9,              0xdd                           },
    { 0xaa,              0xec                           },
    { 0xab,              0x1a                           },

    { 0xce,              0x78                           },
    { 0xcf,              0x6e                           },
    { 0xd0,              0x0a                           },
    { 0xd1,              0x0c                           },
    { 0xd2,              0x84                           },
    { 0xd3,              0x90                           },
    { 0xd4,              0x1e                           },

    { 0x5a,              0x24                           },
    { 0x5b,              0x1f                           },
    { 0x5c,              0x88                           },
    { 0x5d,              0x60                           },

    { 0xac,              0x6e                           },
    { 0xbe,              0xff                           },
    { 0xbf,              0x00                           },

    /* 160x120 */
    { 0x31,              0x28                           }, /* HOUTSIZE MSB */
    { 0x32,              0x3c                           }, /* VOUTSIZE MSB */
    { 0x34,              0x00                           }, /* H/V OUTSIZE LSBs */
    { 0x82,              0x3f                           },
    /* {0x82, 0x01|0x04|0x08|0x10 }, */

    /* YUV */
    { 0x12,              0x00                           },
    { 0x36,              0x3f                           },
    { 0x53,              0x00                           },

    { 0x33,              0x00                           },
    /*    {0x33, 0xc4}, */
    { 0x1b,              0x89                           },
    { 0x22,              0x03                           },

    /* VSYNC, inverse */
    { OV7740_REG28,      OV7740_REG28_VSYNC_NEGATIVE    },

    { OV7740_YUV422CTRL, OV7740_YUV422CTRL_V_FIRST_YUYV },

    { 0xFF,              0xFF                           }
};


/**
 * \brief Addresses and values of the OV7740 registers for the
 * OV7740_QVGA_RGB888 configuration:
 *  - 320*240 pixel by picture (QVGA)
 *  - pixel data in RGB format (8-8-8)
 */
const struct ov_reg OV7740_QVGA_RGB888[] =
{
    { 0x0e,         0x00                           },

    { 0x12,         0x80                           },
    { 0x13,         0x00                           },

    { 0x55,         0x40                           },

    /**************************************************************/
    /*  30fps  11 01 ;clock_divider ;sysclk=24MHz at XCLK=24MHz   */
    /*  20fps  11 02 ;clock_divider ;sysclk=16MHz at XCLK=24MHz   */
    /*  15fps  11 03 ;clock_divider ;sysclk=12MHz at XCLK=24MHz   */
    /*  10fps  11 05 ;sysclk=8MHz at XCLK=24MHz                   */
    /*  7p5fps 11 07 ;sysclk=6MHz at XCLK=24MHz                   */
    /**************************************************************/
    /*  7p5fps 11 0x0A ;sysclk=6MHz at XCLK=16.5MHz  (MCK=132/8)  */
    /**************************************************************/

    { OV7740_CLK,   FRAME_RATE_30                  },

    { 0x12,         0x00                           },
    { 0xd5,         0x10                           },
    { OV7740_REG0C, OV7740_REG0C_MAX_EXPOSURE( 2 ) },
    { 0x0d,         0x34                           },
    { 0x17,         0x25                           },
    { 0x18,         0xa0                           },
    { 0x19,         0x03                           },
    { 0x1a,         0xf0                           },
    { 0x1b,         0x89                           },
    { 0x22,         0x03                           },
    { 0x29,         0x18                           },
    { 0x2b,         0xf8                           },
    { 0x2c,         0x01                           },
    { 0x31,         0xa0                           },
    { 0x32,         0xf0                           },
    { 0x33,         0xc4                           },
    { 0x3a,         0xb4                           },
    { 0x36,         0x3f                           },

    { 0x04,         0x60                           },
    { 0x27,         0x80                           },
    { 0x3d,         0x0f                           },
    { 0x3e,         0x80                           },
    { 0x3f,         0x40                           },
    { 0x40,         0x7f                           },
    { 0x41,         0x6a                           },
    { 0x42,         0x29                           },
    { 0x44,         0xe5                           },
    { 0x45,         0x41                           },
    { 0x47,         0x02                           },
    { 0x49,         0x64                           },
    { 0x4a,         0xa1                           },
    { 0x4b,         0x70                           },
    { 0x4c,         0x1a                           },
    { 0x4d,         0x50                           },
    { 0x4e,         0x13                           },
    { 0x64,         0x00                           },
    { 0x67,         0x88                           },
    { 0x68,         0x1a                           },

    { 0x14,         0x38                           },
    { 0x24,         0x3c                           },
    { 0x25,         0x30                           },
    { 0x26,         0x72                           },
    { 0x50,         0x97                           },
    { 0x51,         0x7e                           },
    { 0x52,         0x00                           },
    { 0x53,         0x00                           },
    { 0x20,         0x00                           },
    { 0x21,         0x23                           },
    { 0x38,         0x14                           },
    { 0xe9,         0x00                           },
    { 0x56,         0x55                           },
    { 0x57,         0xff                           },
    { 0x58,         0xff                           },
    { 0x59,         0xff                           },
    { 0x5f,         0x04                           },
    { 0xec,         0x00                           },
    { 0x13,         0xff                           },

    { 0x80,         0x7f                           },
    { 0x81,         0x3f                           },
    { 0x82,         0x32                           },
    { 0x83,         0x01                           },
    { 0x38,         0x11                           },
    { 0x84,         0x70                           },
    { 0x85,         0x00                           },
    { 0x86,         0x03                           },
    { 0x87,         0x01                           },
    { 0x88,         0x05                           },
    { 0x89,         0x30                           },
    { 0x8d,         0x30                           },
    { 0x8f,         0x85                           },
    { 0x93,         0x30                           },
    { 0x95,         0x85                           },
    { 0x99,         0x30                           },
    { 0x9b,         0x85                           },

    { 0x9c,         0x08                           },
    { 0x9d,         0x12                           },
    { 0x9e,         0x23                           },
    { 0x9f,         0x45                           },
    { 0xa0,         0x55                           },
    { 0xa1,         0x64                           },
    { 0xa2,         0x72                           },
    { 0xa3,         0x7f                           },
    { 0xa4,         0x8b                           },
    { 0xa5,         0x95                           },
    { 0xa6,         0xa7                           },
    { 0xa7,         0xb5                           },
    { 0xa8,         0xcb                           },
    { 0xa9,         0xdd                           },
    { 0xaa,         0xec                           },
    { 0xab,         0x1a                           },

    { 0xce,         0x78                           },
    { 0xcf,         0x6e                           },
    { 0xd0,         0x0a                           },
    { 0xd1,         0x0c                           },
    { 0xd2,         0x84                           },
    { 0xd3,         0x90                           },
    { 0xd4,         0x1e                           },

    { 0x5a,         0x24                           },
    { 0x5b,         0x1f                           },
    { 0x5c,         0x88                           },
    { 0x5d,         0x60                           },

    { 0xac,         0x6e                           },
    { 0xbe,         0xff                           },
    { 0xbf,         0x00                           },

    /* 320x240 */
    { 0x31,         0x50                           },
    { 0x32,         0x78                           },
    { 0x82,         0x3f                           },

    /* VGA,RGBRAW_8 */
    { 0x12,         0x01                           },
    { 0x36,         0x2f                           },
    { 0x83,         0x04                           },
    { 0x53,         0x00                           },

    { 0x33,         0xf4                           },
    { 0x1b,         0x8a                           },
    { 0x22,         0x03                           },

    /*  */
    { 0x84,         0x00                           },
    { 0x84,         0x00                           },

    { 0x28,         0x02                           },

    /*  */
    { 0xFF,         0xFF                           },
};


/**
 * \brief Addresses and values of the OV7740 registers for the
 * OV7740_QVGA_RGB888 configuration:
 *  - 160*120 pixel by picture (QQVGA)
 *  - pixel data in RGB format (8-8-8)
 */
const struct ov_reg OV7740_QQVGA_RGB888[] =
{
    { 0x0e,         0x00                                                          },

    { 0x12,         0x80                                                          },
    { 0x13,         0x00                                                          },

    { 0x55,         0x40                                                          },

    /**************************************************************/
    /*  30fps  11 01 ;clock_divider ;sysclk=24MHz at XCLK=24MHz   */
    /*  20fps  11 02 ;clock_divider ;sysclk=16MHz at XCLK=24MHz   */
    /*  15fps  11 03 ;clock_divider ;sysclk=12MHz at XCLK=24MHz   */
    /*  10fps  11 05 ;sysclk=8MHz at XCLK=24MHz                   */
    /*  7p5fps 11 07 ;sysclk=6MHz at XCLK=24MHz                   */
    /**************************************************************/
    /*  7p5fps 11 0x0A ;sysclk=6MHz at XCLK=16.5MHz  (MCK=132/8)  */
    /**************************************************************/

    { OV7740_CLK,   FRAME_RATE_30                                                 },

    { 0x12,         0x00                                                          },
    { 0xd5,         0x10                                                          },
    { OV7740_REG0C, ( OV7740_REG0C_FLIP_ENABLE | OV7740_REG0C_MAX_EXPOSURE( 2 ) ) },
    { 0x0d,         0x34                                                          },
    { 0x17,         0x25                                                          },
    { 0x18,         0xa0                                                          },
    { 0x19,         0x03                                                          },
    { 0x1a,         0xf0                                                          },
    { 0x1b,         0x89                                                          },
    { 0x22,         0x03                                                          },
    { 0x29,         0x18                                                          },
    { 0x2b,         0xf8                                                          },
    { 0x2c,         0x01                                                          },
    { 0x31,         0xa0                                                          },
    { 0x32,         0xf0                                                          },
    { 0x33,         0xc4                                                          },
    { 0x3a,         0xb4                                                          },
    { 0x36,         0x3f                                                          },

    { 0x04,         0x60                                                          },
    { 0x27,         0x80                                                          },
    { 0x3d,         0x0f                                                          },
    { 0x3e,         0x80                                                          },
    { 0x3f,         0x40                                                          },
    { 0x40,         0x7f                                                          },
    { 0x41,         0x6a                                                          },
    { 0x42,         0x29                                                          },
    { 0x44,         0xe5                                                          },
    { 0x45,         0x41                                                          },
    { 0x47,         0x02                                                          },
    { 0x49,         0x64                                                          },
    { 0x4a,         0xa1                                                          },
    { 0x4b,         0x70                                                          },
    { 0x4c,         0x1a                                                          },
    { 0x4d,         0x50                                                          },
    { 0x4e,         0x13                                                          },
    { 0x64,         0x00                                                          },
    { 0x67,         0x88                                                          },
    { 0x68,         0x1a                                                          },

    { 0x14,         0x38                                                          },
    { 0x24,         0x3c                                                          },
    { 0x25,         0x30                                                          },
    { 0x26,         0x72                                                          },
    { 0x50,         0x97                                                          },
    { 0x51,         0x7e                                                          },
    { 0x52,         0x00                                                          },
    { 0x53,         0x00                                                          },
    { 0x20,         0x00                                                          },
    { 0x21,         0x23                                                          },
    { 0x38,         0x14                                                          },
    { 0xe9,         0x00                                                          },
    { 0x56,         0x55                                                          },
    { 0x57,         0xff                                                          },
    { 0x58,         0xff                                                          },
    { 0x59,         0xff                                                          },
    { 0x5f,         0x04                                                          },
    { 0xec,         0x00                                                          },
    { 0x13,         0xff                                                          },

    { 0x80,         0x7f                                                          },
    { 0x81,         0x3f                                                          },
    { 0x82,         0x32                                                          },
    { 0x83,         0x01                                                          },
    { 0x38,         0x11                                                          },
    { 0x84,         0x70                                                          },
    { 0x85,         0x00                                                          },
    { 0x86,         0x03                                                          },
    { 0x87,         0x01                                                          },
    { 0x88,         0x05                                                          },
    { 0x89,         0x30                                                          },
    { 0x8d,         0x30                                                          },
    { 0x8f,         0x85                                                          },
    { 0x93,         0x30                                                          },
    { 0x95,         0x85                                                          },
    { 0x99,         0x30                                                          },
    { 0x9b,         0x85                                                          },

    { 0x9c,         0x08                                                          },
    { 0x9d,         0x12                                                          },
    { 0x9e,         0x23                                                          },
    { 0x9f,         0x45                                                          },
    { 0xa0,         0x55                                                          },
    { 0xa1,         0x64                                                          },
    { 0xa2,         0x72                                                          },
    { 0xa3,         0x7f                                                          },
    { 0xa4,         0x8b                                                          },
    { 0xa5,         0x95                                                          },
    { 0xa6,         0xa7                                                          },
    { 0xa7,         0xb5                                                          },
    { 0xa8,         0xcb                                                          },
    { 0xa9,         0xdd                                                          },
    { 0xaa,         0xec                                                          },
    { 0xab,         0x1a                                                          },

    { 0xce,         0x78                                                          },
    { 0xcf,         0x6e                                                          },
    { 0xd0,         0x0a                                                          },
    { 0xd1,         0x0c                                                          },
    { 0xd2,         0x84                                                          },
    { 0xd3,         0x90                                                          },
    { 0xd4,         0x1e                                                          },

    { 0x5a,         0x24                                                          },
    { 0x5b,         0x1f                                                          },
    { 0x5c,         0x88                                                          },
    { 0x5d,         0x60                                                          },

    { 0xac,         0x6e                                                          },
    { 0xbe,         0xff                                                          },
    { 0xbf,         0x00                                                          },

    /* 160x120 */
    { 0x31,         0x28                                                          },
    { 0x32,         0x3c                                                          },
    { 0x82,         0x3f                                                          },

    /* VGA, RGBRAW_8 */
    { 0x12,         0x01                                                          },
    { 0x36,         0x2f                                                          },
    { 0x83,         0x04                                                          },
    { 0x53,         0x00                                                          },

    { 0x33,         0xf4                                                          },
    { 0x1b,         0x8a                                                          },
    { 0x22,         0x03                                                          },

    { 0x84,         0x00                                                          },
    { 0x84,         0x00                                                          },

    { 0x28,         0x02                                                          },

    { 0xFF,         0xFF                                                          },
};

/**
 * \brief Addresses and values of the OV7740 registers for the
 * OV7740_QQVGA_YUV422 configuration:
 *  - 352*288 pixel by picture (CIF)
 *  - pixel data in YUV422 format (Y1, U, Y2, V)
 */
const struct ov_reg OV7740_CIF_YUV422[] =
{
    { OV7740_REG0E,      OV7740_REG0E_OUTPUT_1X         },
    { OV7740_REG12,      OV7740_REG12_RESET             },
    { 0x13,              0x00                           },
    { 0x55,              0x40                           },

    /**************************************************************/
    /*  30fps  11 01 ;clock_divider ;sysclk=24MHz at XCLK=24MHz   */
    /*  20fps  11 02 ;clock_divider ;sysclk=16MHz at XCLK=24MHz   */
    /*  15fps  11 03 ;clock_divider ;sysclk=12MHz at XCLK=24MHz   */
    /*  10fps  11 05 ;sysclk=8MHz at XCLK=24MHz                   */
    /*  7p5fps 11 07 ;sysclk=6MHz at XCLK=24MHz                   */
    /**************************************************************/
    /*  7p5fps 11 0x0A ;sysclk=6MHz at XCLK=16.5MHz  (MCK=132/8)  */
    /**************************************************************/

    { OV7740_CLK,        FRAME_RATE_30                  },
    { 0x12,              0x00                           },
    { 0xd5,              0x10                           },
    { OV7740_REG0C,
      ( OV7740_REG0C_MIRROR_ENABLE | OV7740_REG0C_MAX_EXPOSURE( 2 )
        | OV7740_REG0C_YUV_SWAP_ENABLE ) },
    { 0x0d,              0x34                           },
    { 0x16,              0x01                           },
    { 0x17,              0x25                           },
    { 0x18,              0xa0                           },
    { 0x19,              0x03                           },
    { 0x1a,              0xf0                           },
    { 0x1b,              0x89                           },
    { 0x22,              0x03                           },
    { 0x29,              0x18                           },
    { 0x2b,              0xf8                           },
    { 0x2c,              0x01                           },
    { 0x31,              0xa0                           },
    { 0x32,              0xf0                           },
    { 0x33,              0xc4                           },
    { 0x3a,              0xb4                           },
    { 0x36,              0x3f                           },

    { 0x04,              0x60                           },
    { 0x27,              0x80                           },
    { 0x3d,              0x0f                           },
    { 0x3e,              0x80                           },
    { 0x3f,              0x40                           },
    { 0x40,              0x7f                           },
    { 0x41,              0x6a                           },
    { 0x42,              0x29                           },
    { 0x44,              0xe5                           },
    { 0x45,              0x41                           },
    { 0x47,              0x02                           },
    { 0x49,              0x64                           },
    { 0x4a,              0xa1                           },
    { 0x4b,              0x70                           },
    { 0x4c,              0x1a                           },
    { 0x4d,              0x50                           },
    { 0x4e,              0x13                           },
    { 0x64,              0x00                           },
    { 0x67,              0x88                           },
    { 0x68,              0x1a                           },

    { 0x14,              0x38                           },
    { 0x24,              0x3c                           },
    { 0x25,              0x30                           },
    { 0x26,              0x72                           },
    { 0x50,              0x97                           },
    { 0x51,              0x7e                           },
    { 0x52,              0x00                           },
    { 0x53,              0x00                           },
    { 0x20,              0x00                           },
    { 0x21,              0x23                           },
    { 0x38,              0x14                           },
    { 0xe9,              0x00                           },
    { 0x56,              0x55                           },
    { 0x57,              0xff                           },
    { 0x58,              0xff                           },
    { 0x59,              0xff                           },
    { 0x5f,              0x04                           },
    { 0xec,              0x00                           },
    { 0x13,              0xff                           },

    { 0x80,              0x7f                           },
    { 0x81,              0x3f                           },
    { 0x82,              0x32                           },
    { 0x83,              0x01                           },
    { 0x38,              0x11                           },
    { 0x84,              0x70                           },
    { 0x85,              0x00                           },
    { 0x86,              0x03                           },
    { 0x87,              0x01                           },
    { 0x88,              0x05                           },
    { 0x89,              0x30                           },
    { 0x8d,              0x30                           },
    { 0x8f,              0x85                           },
    { 0x93,              0x30                           },
    { 0x95,              0x85                           },
    { 0x99,              0x30                           },
    { 0x9b,              0x85                           },

    { 0x9c,              0x08                           },
    { 0x9d,              0x12                           },
    { 0x9e,              0x23                           },
    { 0x9f,              0x45                           },
    { 0xa0,              0x55                           },
    { 0xa1,              0x64                           },
    { 0xa2,              0x72                           },
    { 0xa3,              0x7f                           },
    { 0xa4,              0x8b                           },
    { 0xa5,              0x95                           },
    { 0xa6,              0xa7                           },
    { 0xa7,              0xb5                           },
    { 0xa8,              0xcb                           },
    { 0xa9,              0xdd                           },
    { 0xaa,              0xec                           },
    { 0xab,              0x1a                           },

    { 0xce,              0x78                           },
    { 0xcf,              0x6e                           },
    { 0xd0,              0x0a                           },
    { 0xd1,              0x0c                           },
    { 0xd2,              0x84                           },
    { 0xd3,              0x90                           },
    { 0xd4,              0x1e                           },

    { 0x5a,              0x24                           },
    { 0x5b,              0x1f                           },
    { 0x5c,              0x88                           },
    { 0x5d,              0x60                           },

    { 0xac,              0x6e                           },
    { 0xbe,              0xff                           },
    { 0xbf,              0x00                           },

    /* 352x288 */
    { 0x31,              0x58                           }, /* HOUTSIZE MSB */
    { 0x32,              0x90                           }, /* VOUTSIZE MSB */
    { 0x34,              0x00                           }, /* H/V OUTSIZE LSBs */
    { 0x82,              0x3f                           },
    /* {0x82, 0x01|0x04|0x08|0x10 }, */

    /* YUV */
    { 0x12,              0x00                           },
    { 0x36,              0x3f                           },
    { 0x53,              0x00                           },

    { 0x33,              0x00                           },
    /*    {0x33, 0xc4}, */
    { 0x1b,              0x89                           },
    { 0x22,              0x03                           },

    /* VSYNC, inverse */
    { OV7740_REG28,      OV7740_REG28_VSYNC_NEGATIVE    },

    { OV7740_YUV422CTRL, OV7740_YUV422CTRL_V_FIRST_YUYV },

    { 0xFF,              0xFF                           }
};

/**
 * \brief Addresses and values of the OV7740 registers for the
 * OV7740_TEST_PATTERN configuration:
 *  - 320*240 pixel by picture (QVGA)
 *  - pixel data in YUV422 format (Y1, U, Y2, V)
 *  - 20 frames per second
 *  - test pattern enable
 */
const struct ov_reg OV7740_TEST_PATTERN[] =
{
    { 0x0e,         0x00          },

    { 0x12,         0x80          },
    /* flag for soft reset delay */
    { 0xFE,         0x05          },
    { 0x13,         0x00          },
    { 0x55,         0x40          },

    /**************************************************************/
    /*  30fps  11 01 ;clock_divider ;sysclk=24MHz at XCLK=24MHz   */
    /*  20fps  11 02 ;clock_divider ;sysclk=16MHz at XCLK=24MHz   */
    /*  15fps  11 03 ;clock_divider ;sysclk=12MHz at XCLK=24MHz   */
    /*  10fps  11 05 ;sysclk=8MHz at XCLK=24MHz                   */
    /*  7p5fps 11 07 ;sysclk=6MHz at XCLK=24MHz                   */
    /**************************************************************/
    /*  7p5fps 11 0x0A ;sysclk=6MHz at XCLK=16.5MHz  (MCK=132/8)  */
    /**************************************************************/

    { OV7740_CLK,   FRAME_RATE_30 },

    { 0x12,         0x00          },
    { 0xd5,         0x10          },
    { OV7740_REG0C,
      ( OV7740_REG0C_MIRROR_ENABLE | OV7740_REG0C_FLIP_ENABLE
        | OV7740_REG0C_MAX_EXPOSURE( 2 ) | OV7740_REG0C_YUV_SWAP_ENABLE ) },
    { 0x0d,         0x34          },
    { 0x16,         0x01          },
    { 0x17,         0x25          },
    { 0x18,         0xa0          },
    { 0x19,         0x03          },
    { 0x1a,         0xf0          },
    { 0x1b,         0x89          },
    { 0x22,         0x03          },
    { 0x29,         0x18          },
    { 0x2b,         0xf8          },
    { 0x2c,         0x01          },
    { 0x31,         0xa0          },
    { 0x32,         0xf0          },
    { 0x33,         0xc4          },
    { 0x3a,         0xb4          },
    { 0x36,         0x3f          },

    { 0x04,         0x60          },
    { 0x27,         0x80          },
    { 0x3d,         0x0f          },
    { 0x3e,         0x80          },
    { 0x3f,         0x40          },
    { 0x40,         0x7f          },
    { 0x41,         0x6a          },
    { 0x42,         0x29          },
    { 0x44,         0xe5          },
    { 0x45,         0x41          },
    { 0x47,         0x02          },
    { 0x49,         0x64          },
    { 0x4a,         0xa1          },
    { 0x4b,         0x70          },
    { 0x4c,         0x1a          },
    { 0x4d,         0x50          },
    { 0x4e,         0x13          },
    { 0x64,         0x00          },
    { 0x67,         0x88          },
    { 0x68,         0x1a          },

    { 0x14,         0x38          },
    { 0x24,         0x3c          },
    { 0x25,         0x30          },
    { 0x26,         0x72          },
    { 0x50,         0x97          },
    { 0x51,         0x7e          },
    { 0x52,         0x00          },
    { 0x53,         0x00          },
    { 0x20,         0x00          },
    { 0x21,         0x23          },
    /*********************************/
    /* To enable Static Test Pattern */
    /*********************************/
    { 0x38,         0x07          },
    { 0x84,         0x02          },
    /*********************************/
    /* Normal Mode */
    { 0x38,         0x14          },
    /*********************************/
    { 0xe9,         0x00          },
    { 0x56,         0x55          },
    { 0x57,         0xff          },
    { 0x58,         0xff          },
    { 0x59,         0xff          },
    { 0x5f,         0x04          },
    { 0xec,         0x00          },
    { 0x13,         0xff          },

    { 0x80,         0x7f          },
    { 0x81,         0x3f          },
    { 0x82,         0x32          },
    { 0x83,         0x01          },
    { 0x38,         0x11          },
    { 0x85,         0x00          },
    { 0x86,         0x03          },
    { 0x87,         0x01          },
    { 0x88,         0x05          },
    { 0x89,         0x30          },
    { 0x8d,         0x30          },
    { 0x8f,         0x85          },
    { 0x93,         0x30          },
    { 0x95,         0x85          },
    { 0x99,         0x30          },
    { 0x9b,         0x85          },

    { 0x9c,         0x08          },
    { 0x9d,         0x12          },
    { 0x9e,         0x23          },
    { 0x9f,         0x45          },
    { 0xa0,         0x55          },
    { 0xa1,         0x64          },
    { 0xa2,         0x72          },
    { 0xa3,         0x7f          },
    { 0xa4,         0x8b          },
    { 0xa5,         0x95          },
    { 0xa6,         0xa7          },
    { 0xa7,         0xb5          },
    { 0xa8,         0xcb          },
    { 0xa9,         0xdd          },
    { 0xaa,         0xec          },
    { 0xab,         0x1a          },

    { 0xce,         0x78          },
    { 0xcf,         0x6e          },
    { 0xd0,         0x0a          },
    { 0xd1,         0x0c          },
    { 0xd2,         0x84          },
    { 0xd3,         0x90          },
    { 0xd4,         0x1e          },

    { 0x5a,         0x24          },
    { 0x5b,         0x1f          },
    { 0x5c,         0x88          },
    { 0x5d,         0x60          },

    { 0xac,         0x6e          },
    { 0xbe,         0xff          },
    { 0xbf,         0x00          },

    /* 320x240 */

    { 0x31,         0x50          },
    { 0x32,         0x78          },
    { 0x82,         0x3f          },

    /* YUV */
    { 0x12,         0x00          },
    { 0x36,         0x3f          },
    { 0x53,         0x00          },

    { 0x33,         0xc4          },
    { 0x1b,         0x89          },
    { 0x22,         0x03          },

    /* VSYNC, inverse */
    { 0x28,         0x2           },

    { 0xFF,         0xFF          }
};

/* @} */

#ifdef __cplusplus
}
#endif
