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

/*------------------------------------------------------------------------------
 *         Headers
 *------------------------------------------------------------------------------*/

#include "board.h"

/*
 * ID
 */



/*------------------------------------------------------------------------------
 *         Local Variables
 *------------------------------------------------------------------------------*/

const struct ov_reg ov2640_yuv_vga[] =
{
    { 0xff, 0x01               }, /*dsp */
    { 0x12, 0x80               }, /*reset */
    { 0xff, 0x00               }, /*sensor */
    { 0x2c, 0xff               }, /*? */
    { 0x2e, 0xdf               }, /*ADDVSH, VSYNC msb=223 */
    { 0xff, 0x01               }, /*dsp */
    { 0x3c, 0x32               }, /*? */
    { 0x11, 0x00               }, /*clock rate off */
    { 0x09, 0x02               }, /*2 capablity + standby mode */
    { 0x04, 0x28               }, /*? ?????????????????????????????????? */
    { 0x13, 0xe5               }, /* */
    { 0x14, 0x48               }, /*Auto agc */
    { 0x2c, 0x0c               }, /*? */
    { 0x33, 0x78               }, /*? */
    { 0x3a, 0x33               }, /*? */
    { 0x3b, 0xfb               }, /*? */
    { 0x3e, 0x00               }, /*? */
    { 0x43, 0x11               }, /*? */
    { 0x16, 0x10               }, /*? */
    { 0x39, 0x02               }, /*? */
    { 0x35, 0x88               }, /*? */
    { 0x22, 0x0a               }, /*? */
    { 0x37, 0x40               }, /*? */
    { 0x23, 0x00               }, /*? */
    { 0x34, 0xa0               }, /*startpoint 0 */
    { 0x36, 0x1a               }, /*? XXXXXXXXXXXXXXXX */
    { 0x06, 0x02               }, /*? */
    { 0x07, 0xc0               }, /*? */
    { 0x0d, 0xb7               }, /*? */
    { 0x0e, 0x01               }, /*? */
    { 0x4c, 0x00               }, /*? */
    { 0x4a, 0x81               }, /*? */
    { 0x21, 0x99               }, /*? */
    { 0x24, 0x3a               }, /* Luminance high */
    { 0x25, 0x32               }, /* Luminance low */
    /*{0x24, 0x10}, // Luminance high */
    /*{0x25, 0x03}, // Luminance low */

    { 0x26, 0xF3               }, /* Fast mode large Step Range Threshold */
    { 0x5c, 0x00               }, /*? */
    { 0x63, 0x00               }, /*? */
    { 0x5d, 0x55               }, /*zone */
    { 0x5e, 0x7d               }, /*zone */
    { 0x5f, 0x7d               }, /*zone */
    { 0x60, 0x55               }, /*zone */
    { 0x61, 0x70               }, /*Histogram low */
    { 0x62, 0x80               }, /*Histogram high */
    { 0x7c, 0x05               }, /*? */
    { 0x20, 0x80               }, /*? */
    { 0x28, 0x30               }, /*? */
    { 0x6c, 0x00               }, /*? */
    { 0x6d, 0x80               }, /*? */
    { 0x6e, 0x00               }, /*? */
    { 0x70, 0x02               }, /*? */
    { 0x71, 0x94               }, /*? */
    { 0x73, 0xc1               }, /*? */
    { 0x3d, 0x34               }, /*? */
    { 0x5a, 0x57               }, /*? */
    { 0x4f, 0xbb               }, /*50Hz */
    { 0x50, 0x9c               }, /*60Hz */

    { 0xff, 0x00               }, /*dsp */
    { 0xe5, 0x7f               }, /*? */
    { 0xf9, 0xc0               }, /*MicroC reset,Boot */
    { 0x41, 0x24               }, /*? */
    { 0xe0, 0x14               }, /*JPEG,DVP reset */
    { 0x76, 0xff               }, /*? */
    { 0x33, 0xa0               }, /*? */
    { 0x42, 0x20               }, /*? */
    { 0x43, 0x18               }, /*? */
    { 0x4c, 0x00               }, /*? */
    { 0x87, 0xd0               }, /*Module Enable BPC+WPC 11010000 */
    { 0x88, 0x3f               }, /*? */
    { 0xd7, 0x03               }, /*? */
    { 0xd9, 0x10               }, /*? */
    { 0xd3, 0x82               }, /*Auto mode */
    { 0xc8, 0x08               }, /*? */
    { 0xc9, 0x80               }, /*? */
    { 0x7c, 0x00               }, /*SDE indirect register access: address */
    { 0x7d, 0x02               }, /*SDE indirect register data */
    { 0x7c, 0x03               }, /* */
    { 0x7d, 0x48               }, /* */
    { 0x7d, 0x48               }, /* */
    { 0x7c, 0x08               }, /* */
    { 0x7d, 0x20               }, /* */
    { 0x7d, 0x10               }, /* */
    { 0x7d, 0x0e               }, /* */
    { 0x90, 0x00               }, /*? */
    { 0x91, 0x0e               }, /*? */
    { 0x91, 0x1a               }, /*? */
    { 0x91, 0x31               }, /*? */
    { 0x91, 0x5a               }, /*? */
    { 0x91, 0x69               }, /*? */
    { 0x91, 0x75               }, /*? */
    { 0x91, 0x7e               }, /*? */
    { 0x91, 0x88               }, /*? */
    { 0x91, 0x8f               }, /*? */
    { 0x91, 0x96               }, /*? */
    { 0x91, 0xa3               }, /*? */
    { 0x91, 0xaf               }, /*? */
    { 0x91, 0xc4               }, /*? */
    { 0x91, 0xd7               }, /*? */
    { 0x91, 0xe8               }, /*? */
    { 0x91, 0x20               }, /*? */
    { 0x92, 0x00               }, /*? */
    { 0x93, 0x06               }, /*? */
    { 0x93, 0xe3               }, /*? */
    { 0x93, 0x05               }, /*? */
    { 0x93, 0x05               }, /*? */
    { 0x93, 0x00               }, /*? */
    { 0x93, 0x02               }, /*? */
    { 0x93, 0x00               }, /*? */
    { 0x93, 0x00               }, /*? */
    { 0x93, 0x00               }, /*? */
    { 0x93, 0x00               }, /*? */
    { 0x93, 0x00               }, /*? */
    { 0x93, 0x00               }, /*? */
    { 0x93, 0x00               }, /*? */
    { 0x96, 0x00               }, /*? */
    { 0x97, 0x08               }, /*? */
    { 0x97, 0x19               }, /*? */
    { 0x97, 0x02               }, /*? */
    { 0x97, 0x0c               }, /*? */
    { 0x97, 0x24               }, /*? */
    { 0x97, 0x30               }, /*? */
    { 0x97, 0x28               }, /*? */
    { 0x97, 0x26               }, /*? */
    { 0x97, 0x02               }, /*? */
    { 0x97, 0x98               }, /*? */
    { 0x97, 0x80               }, /*? */
    { 0x97, 0x00               }, /*? */
    { 0x97, 0x00               }, /*? */
    { 0xc3, 0xed               }, /*Module enable */
    { 0xa4, 0x00               }, /*? */
    { 0xa8, 0x00               }, /*? */
    { 0xc5, 0x11               }, /*? */
    { 0xc6, 0x51               }, /*? */
    { 0xbf, 0x80               }, /*? */
    { 0xc7, 0x10               }, /*? */
    { 0xb6, 0x66               }, /*? */
    { 0xb8, 0xa5               }, /*? */
    { 0xb7, 0x64               }, /*? */
    { 0xb9, 0x7c               }, /*? */
    { 0xb3, 0xaf               }, /*? */
    { 0xb4, 0x97               }, /*? */
    { 0xb5, 0xff               }, /*? */
    { 0xb0, 0xc5               }, /*? */
    { 0xb1, 0x94               }, /*? */
    { 0xb2, 0x0f               }, /*? */
    { 0xc4, 0x5c               }, /*? */
    { 0xc0, 0xc8               }, /* HSIZE8[7:0] 1600 */
    { 0xc1, 0x96               }, /* VSIZE8[7:0] 1200 */
    { 0x86, 0x1d               }, /*Module enable */
    { 0x50, 0x00               }, /*? */
    { 0x51, 0x90               }, /*H_SIZE[7:0] (real/4) 1600 */
    { 0x52, 0x18               }, /*V_SIZE[7:0] (real/4) 1120 */
    { 0x53, 0x00               }, /*OFFSET_X[7:0] */
    { 0x54, 0x00               }, /*OFFSET_Y[7:0] */
    { 0x55, 0x88               }, /*V_SIZE[8]=1 H_SIZE[8] */
    { 0x57, 0x00               }, /*? */
    { 0x5a, 0x90               }, /*OUTW */
    { 0x5b, 0x18               }, /*OUTH */
    { 0x5c, 0x05               }, /*OUTW8 ,OUTH8 */
    { 0xc3, 0xed               }, /* */
    { 0x7f, 0x00               }, /*? */
    { 0xda, 0x04               }, /*Image output format select ------ RAW */
    { 0xe5, 0x1f               }, /*? */
    { 0xe1, 0x67               }, /*? */
    { 0xe0, 0x00               }, /*Reset */
    { 0xdd, 0xff               }, /*? */
    { 0x05, 0x00               }, /*Bypass DSP no */
    { 0xC2, 0x08 | 0x04 | 0x02 },

    { 0xff, 0x01               }, /*Sensor */
    { 0x11, 0x01               }, /*? */
    { 0xff, 0x01               }, /*Sensor */
    { 0x12, 0x40               }, /*Preview mode */
    { 0x17, 0x11               }, /*? */
    { 0x18, 0x43               }, /*? */
    { 0x19, 0x00               }, /*? */
    { 0x1a, 0x4b               }, /*? */
    { 0x32, 0x09               }, /*? */
    { 0x4f, 0xca               }, /*? */
    { 0x50, 0xa8               }, /*10 101 000 V_DIVDER = 5 */
    { 0x5a, 0x23               }, /* OUTW 23 */
    { 0x6d, 0x00               }, /*? */
    { 0x3d, 0x38               }, /*? */
    { 0x39, 0x12               }, /*? */
    { 0x35, 0xda               }, /*? */
    { 0x22, 0x1a               }, /*? */
    { 0x37, 0xc3               }, /*? */
    { 0x23, 0x00               }, /*? */
    { 0x34, 0xc0               }, /*? */
    { 0x36, 0x1a               }, /*? */
    { 0x06, 0x88               }, /*? */
    { 0x07, 0xc0               }, /*? */
    { 0x0d, 0x87               }, /*? */
    { 0x0e, 0x41               }, /*? */
    { 0x4c, 0x00               }, /*? */
    { 0x48, 0x00               }, /*? */
    { 0x5B, 0x00               }, /*OUTH */
    { 0x42, 0x03               }, /*? */
    { 0xff, 0x00               }, /*DSP */

    { 0xe0, 0x04               }, /*Reset DVP */
    { 0xc0, 0x64               }, /* HSIZE8[7:0] 400 */
    { 0xc1, 0x4B               }, /* VSIZE8[7:0] 300 */
    { 0x8c, 0x00               }, /*? */
    { 0x86, 0x1D               }, /*Modle enable */
    { 0xd3, 0x82               }, /*Auto mode DVP PCLK=2 */
    { 0xe0, 0x00               }, /*Reset */

    { 0xff, 0x00               }, /*DSP */
    { 0xc0, 0x64               }, /* HSIZE8[7:0] 400 */
    { 0xc1, 0x4B               }, /* VSIZE8[7:0] 300 */
    { 0x8c, 0x00               }, /*? */
    { 0x86, 0x3D               }, /*? */
    { 0x50, 0x00               }, /*? */
    { 0x51, 0xC8               }, /*H_SIZE[7:0] (real/4) 800 */
    { 0x52, 0x96               }, /*V_SIZE[7:0] (real/4) 600 */
    { 0x53, 0x00               }, /*OFFSET */
    { 0x54, 0x00               }, /*OFFSET */
    { 0x55, 0x00               }, /*H_SIZE[8],V_SIZE[8] */
    { 0x5a, 0xA0               }, /*OUTW[0-7] 160? */
    { 0x5b, 0x78               }, /*OUTH[0-7] 120? */
    { 0x5c, 0x00               }, /*OUTW8,OUTH8 */
    { 0xd3, 0x04               }, /*? */
    { 0xFF, 0x00               },
    { 0xE0, 0x04               },
    { 0xE1, 0x67               },
    { 0xD7, 0x01               },
    { 0xDA, 0x00               }, /*Image output format select ------ YUV422 */
    { 0xD3, 0x82               },
    { 0xE0, 0x00               },
    { 0xFF, 0xFF               }
};
