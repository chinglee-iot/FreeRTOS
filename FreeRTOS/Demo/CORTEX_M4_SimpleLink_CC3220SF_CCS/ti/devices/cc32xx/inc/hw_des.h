/*
 * -------------------------------------------
 *    CC3220 SDK - v0.10.00.00
 * -------------------------------------------
 *
 *  Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __HW_DES_H__
#define __HW_DES_H__

/****************************************************************************** */
/* */
/* The following are defines for the DES_P register offsets. */
/* */
/****************************************************************************** */
#define DES_O_KEY3_L    0x00000000        /* KEY3 (LSW) for 192-bit key */
#define DES_O_KEY3_H    0x00000004        /* KEY3 (MSW) for 192-bit key */
#define DES_O_KEY2_L    0x00000008        /* KEY2 (LSW) for 192-bit key */
#define DES_O_KEY2_H    0x0000000C        /* KEY2 (MSW) for 192-bit key */
#define DES_O_KEY1_L    0x00000010        /* KEY1 (LSW) for 128-bit */
                                          /* key/192-bit key */
#define DES_O_KEY1_H    0x00000014        /* KEY1 (LSW) for 128-bit */
                                          /* key/192-bit key */
#define DES_O_IV_L      0x00000018        /* Initialization vector LSW */
#define DES_O_IV_H      0x0000001C        /* Initialization vector MSW */
#define DES_O_CTRL      0x00000020
#define DES_O_LENGTH    0x00000024        /* Indicates the cryptographic data */
                                          /* length in bytes for all modes. */
                                          /* Once processing is started with */
                                          /* this context this length */
                                          /* decrements to zero. Data lengths */
                                          /* up to (2^32 – 1) bytes are */
                                          /* allowed. A write to this register */
                                          /* triggers the engine to start */
                                          /* using this context. For a Host */
                                          /* read operation these registers */
                                          /* return all-zeroes. */
#define DES_O_DATA_L       0x00000028     /* Data register(LSW) to read/write */
                                          /* encrypted/decrypted data. */
#define DES_O_DATA_H       0x0000002C     /* Data register(MSW) to read/write */
                                          /* encrypted/decrypted data. */
#define DES_O_REVISION     0x00000030
#define DES_O_SYSCONFIG    0x00000034
#define DES_O_SYSSTATUS    0x00000038
#define DES_O_IRQSTATUS    0x0000003C     /* This register indicates the */
                                          /* interrupt status. If one of the */
                                          /* interrupt bits is set the */
                                          /* interrupt output will be asserted */
#define DES_O_IRQENABLE    0x00000040     /* This register contains an enable */
                                          /* bit for each unique interrupt */
                                          /* generated by the module. It */
                                          /* matches the layout of */
                                          /* DES_IRQSTATUS register. An */
                                          /* interrupt is enabled when the bit */
                                          /* in this register is set to 1 */



/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_KEY3_L register. */
/* */
/******************************************************************************* */
#define DES_KEY3_L_KEY3_L_M    0xFFFFFFFF /* data for key3 */
#define DES_KEY3_L_KEY3_L_S    0
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_KEY3_H register. */
/* */
/******************************************************************************* */
#define DES_KEY3_H_KEY3_H_M    0xFFFFFFFF /* data for key3 */
#define DES_KEY3_H_KEY3_H_S    0
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_KEY2_L register. */
/* */
/******************************************************************************* */
#define DES_KEY2_L_KEY2_L_M    0xFFFFFFFF /* data for key2 */
#define DES_KEY2_L_KEY2_L_S    0
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_KEY2_H register. */
/* */
/******************************************************************************* */
#define DES_KEY2_H_KEY2_H_M    0xFFFFFFFF /* data for key2 */
#define DES_KEY2_H_KEY2_H_S    0
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_KEY1_L register. */
/* */
/******************************************************************************* */
#define DES_KEY1_L_KEY1_L_M    0xFFFFFFFF /* data for key1 */
#define DES_KEY1_L_KEY1_L_S    0
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_KEY1_H register. */
/* */
/******************************************************************************* */
#define DES_KEY1_H_KEY1_H_M    0xFFFFFFFF /* data for key1 */
#define DES_KEY1_H_KEY1_H_S    0
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_IV_L register. */
/* */
/******************************************************************************* */
#define DES_IV_L_IV_L_M    0xFFFFFFFF     /* initialization vector for CBC */
                                          /* CFB modes */
#define DES_IV_L_IV_L_S    0
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_IV_H register. */
/* */
/******************************************************************************* */
#define DES_IV_H_IV_H_M    0xFFFFFFFF     /* initialization vector for CBC */
                                          /* CFB modes */
#define DES_IV_H_IV_H_S    0
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_CTRL register. */
/* */
/******************************************************************************* */
#define DES_CTRL_CONTEXT    0x80000000      /* If ‘1’ this read-only status bit */
                                            /* indicates that the context data */
                                            /* registers can be overwritten and */
                                            /* the host is permitted to write */
                                            /* the next context. */
#define DES_CTRL_MODE_M          0x00000030 /* Select CBC ECB or CFB mode 0x0 */
                                            /* ecb mode 0x1 cbc mode 0x2 cfb */
                                            /* mode 0x3 reserved */
#define DES_CTRL_MODE_S          4
#define DES_CTRL_TDES            0x00000008 /* Select DES or triple DES */
                                            /* encryption/decryption. 0 des mode */
                                            /* 1 tdes mode */
#define DES_CTRL_DIRECTION       0x00000004 /* select encryption/decryption 0 */
                                            /* decryption is selected 1 */
                                            /* Encryption is selected */
#define DES_CTRL_INPUT_READY     0x00000002 /* When '1' ready to */
                                            /* encrypt/decrypt data */
#define DES_CTRL_OUTPUT_READY    0x00000001 /* When '1' Data */
                                            /* decrypted/encrypted ready */
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_LENGTH register. */
/* */
/******************************************************************************* */
#define DES_LENGTH_LENGTH_M    0xFFFFFFFF
#define DES_LENGTH_LENGTH_S    0
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_DATA_L register. */
/* */
/******************************************************************************* */
#define DES_DATA_L_DATA_L_M    0xFFFFFFFF /* data for encryption/decryption */
#define DES_DATA_L_DATA_L_S    0
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_DATA_H register. */
/* */
/******************************************************************************* */
#define DES_DATA_H_DATA_H_M    0xFFFFFFFF /* data for encryption/decryption */
#define DES_DATA_H_DATA_H_S    0
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_REVISION register. */
/* */
/******************************************************************************* */
#define DES_REVISION_SCHEME_M    0xC0000000
#define DES_REVISION_SCHEME_S    30
#define DES_REVISION_FUNC_M      0x0FFF0000 /* Function indicates a software */
                                            /* compatible module family. If */
                                            /* there is no level of software */
                                            /* compatibility a new Func number */
                                            /* (and hence REVISION) should be */
                                            /* assigned. */
#define DES_REVISION_FUNC_S     16
#define DES_REVISION_R_RTL_M    0x0000F800  /* RTL Version (R) maintained by IP */
                                            /* design owner. RTL follows a */
                                            /* numbering such as X.Y.R.Z which */
                                            /* are explained in this table. R */
                                            /* changes ONLY when: (1) PDS */
                                            /* uploads occur which may have been */
                                            /* due to spec changes (2) Bug fixes */
                                            /* occur (3) Resets to '0' when X or */
                                            /* Y changes. Design team has an */
                                            /* internal 'Z' (customer invisible) */
                                            /* number which increments on every */
                                            /* drop that happens due to DV and */
                                            /* RTL updates. Z resets to 0 when R */
                                            /* increments. */
#define DES_REVISION_R_RTL_S    11
#define DES_REVISION_X_MAJOR_M \
    0x00000700                              /* Major Revision (X) maintained by */
                                            /* IP specification owner. X changes */
                                            /* ONLY when: (1) There is a major */
                                            /* feature addition. An example */
                                            /* would be adding Master Mode to */
                                            /* Utopia Level2. The Func field (or */
                                            /* Class/Type in old PID format) */
                                            /* will remain the same. X does NOT */
                                            /* change due to: (1) Bug fixes (2) */
                                            /* Change in feature parameters. */

#define DES_REVISION_X_MAJOR_S    8
#define DES_REVISION_CUSTOM_M     0x000000C0
#define DES_REVISION_CUSTOM_S     6
#define DES_REVISION_Y_MINOR_M \
    0x0000003F                              /* Minor Revision (Y) maintained by */
                                            /* IP specification owner. Y changes */
                                            /* ONLY when: (1) Features are */
                                            /* scaled (up or down). Flexibility */
                                            /* exists in that this feature */
                                            /* scalability may either be */
                                            /* represented in the Y change or a */
                                            /* specific register in the IP that */
                                            /* indicates which features are */
                                            /* exactly available. (2) When */
                                            /* feature creeps from Is-Not list */
                                            /* to Is list. But this may not be */
                                            /* the case once it sees silicon; in */
                                            /* which case X will change. Y does */
                                            /* NOT change due to: (1) Bug fixes */
                                            /* (2) Typos or clarifications (3) */
                                            /* major functional/feature */
                                            /* change/addition/deletion. Instead */
                                            /* these changes may be reflected */
                                            /* via R S X as applicable. Spec */
                                            /* owner maintains a */
                                            /* customer-invisible number 'S' */
                                            /* which changes due to: (1) */
                                            /* Typos/clarifications (2) Bug */
                                            /* documentation. Note that this bug */
                                            /* is not due to a spec change but */
                                            /* due to implementation. */
                                            /* Nevertheless the spec tracks the */
                                            /* IP bugs. An RTL release (say for */
                                            /* silicon PG1.1) that occurs due to */
                                            /* bug fix should document the */
                                            /* corresponding spec number (X.Y.S) */
                                            /* in its release notes. */

#define DES_REVISION_Y_MINOR_S    0
/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_SYSCONFIG register. */
/* */
/******************************************************************************* */
#define DES_SYSCONFIG_DMA_REQ_CONTEXT_IN_EN \
    0x00000080                              /* If set to ‘1’ the DMA context */
                                            /* request is enabled. 0 Dma */
                                            /* disabled 1 Dma enabled */

#define DES_SYSCONFIG_DMA_REQ_DATA_OUT_EN \
    0x00000040                              /* If set to ‘1’ the DMA output */
                                            /* request is enabled. 0 Dma */
                                            /* disabled 1 Dma enabled */

#define DES_SYSCONFIG_DMA_REQ_DATA_IN_EN \
    0x00000020                              /* If set to ‘1’ the DMA input */
                                            /* request is enabled. 0 Dma */
                                            /* disabled 1 Dma enabled */

/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_SYSSTATUS register. */
/* */
/******************************************************************************* */
#define DES_SYSSTATUS_RESETDONE \
    0x00000001

/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_IRQSTATUS register. */
/* */
/******************************************************************************* */
#define DES_IRQSTATUS_DATA_OUT \
    0x00000004                              /* This bit indicates data output */
                                            /* interrupt is active and triggers */
                                            /* the interrupt output. */

#define DES_IRQSTATUS_DATA_IN    0x00000002 /* This bit indicates data input */
                                            /* interrupt is active and triggers */
                                            /* the interrupt output. */
#define DES_IRQSTATUS_CONTEX_IN \
    0x00000001                              /* This bit indicates context */
                                            /* interrupt is active and triggers */
                                            /* the interrupt output. */

/******************************************************************************* */
/* */
/* The following are defines for the bit fields in the DES_O_IRQENABLE register. */
/* */
/******************************************************************************* */
#define DES_IRQENABLE_M_DATA_OUT \
    0x00000004                              /* If this bit is set to ‘1’ the */
                                            /* secure data output interrupt is */
                                            /* enabled. */

#define DES_IRQENABLE_M_DATA_IN \
    0x00000002                              /* If this bit is set to ‘1’ the */
                                            /* secure data input interrupt is */
                                            /* enabled. */

#define DES_IRQENABLE_M_CONTEX_IN \
    0x00000001                              /* If this bit is set to ‘1’ the */
                                            /* secure context interrupt is */
                                            /* enabled. */



#endif // __HW_DES_H__
