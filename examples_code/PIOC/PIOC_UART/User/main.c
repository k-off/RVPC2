/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/06/05
 * Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 *@Note
 *
 *PC18---RX
 *PC19---TX
 *
 *PIOC_UART is half duplex.
 *
 *
 */

#include "debug.h"
#include "string.h"
#include "PIOC_SFR.h"

void PIOC_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/* Global define */
#define     UART_SFR_ADDR1  ((uint8_t *)&(PIOC->D8_DATA_REG8))
#define     UART_SFR_ADDR2  ((uint8_t *)&(PIOC->D8_DATA_REG16))
#define  R16_DATA_REG24_25  (*((volatile unsigned short *)(PIOC_SFR_BASE+0x38))) // RW/RW, data buffer 24~25

volatile uint8_t  PIOC_TX_FLAG=0;
volatile uint16_t PIOC_TX_RemainLEN=0;
volatile uint8_t  *PIOC_TX_ADDR;
uint32_t PIOC_baudrate=921600;
uint8_t  PIOC_parity=0;
uint8_t  PIOC_stopbits=0;
uint8_t  PIOC_word_lenth=8;
u8 rx_buf[100]={0};
volatile u8 rx=0;

__attribute__((aligned(16)))  const unsigned char PIOC_CODE[] =
{0x00,0x00,0x2B,0x63,0xFF,0x0F,0x00,0x00,0x30,0x00,0x00,0x28,0x07,0x10,0x20,0x02,   /* ..+c....0..(.... */
 0x06,0x10,0x0B,0x41,0x05,0x01,0x01,0x02,0x1F,0x10,0x23,0x5B,0x3A,0x40,0x05,0x02,   /* ...A......#[:@.. */
 0x21,0x0C,0x0F,0x38,0x23,0x53,0x3A,0x48,0x00,0x00,0xB8,0x00,0x05,0x01,0x3A,0x02,   /* !..8#S:H......:. */
 0x1F,0x00,0x3A,0x1E,0x3A,0x1B,0x05,0x02,0x21,0x0C,0x1B,0x38,0x00,0x00,0x00,0x00,   /* ..:.:...!..8.... */
 0x00,0x00,0xB9,0x00,0x05,0x01,0x3A,0x02,0x1F,0x00,0x3A,0x1E,0x3A,0x1B,0x05,0x02,   /* ......:...:.:... */
 0x21,0x0C,0x27,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0xBA,0x00,0x05,0x01,0x3A,0x02,   /* !.'8..........:. */
 0x1F,0x00,0x3A,0x1E,0x3A,0x1B,0x05,0x02,0x21,0x0C,0x33,0x38,0x00,0x00,0x00,0x00,   /* ..:.:...!.38.... */
 0x00,0x00,0xBB,0x00,0x05,0x01,0x3A,0x02,0x1F,0x00,0x3A,0x1E,0x3A,0x1B,0x05,0x02,   /* ......:...:.:... */
 0x21,0x0C,0x3F,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0xBC,0x00,0x05,0x01,0x3A,0x02,   /* !.?8..........:. */
 0x1F,0x00,0x3A,0x1E,0x3A,0x1B,0x00,0x00,0x27,0x55,0x7B,0x60,0x00,0x00,0x05,0x02,   /* ..:.:...'U{`.... */
 0x21,0x0C,0x4F,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0xBD,0x00,0x05,0x01,0x3A,0x02,   /* !.O8..........:. */
 0x1F,0x00,0x3A,0x1E,0x3A,0x1B,0x00,0x00,0x27,0x56,0x7B,0x60,0x00,0x00,0x05,0x02,   /* ..:.:...'V{`.... */
 0x21,0x0C,0x5F,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0xBE,0x00,0x05,0x01,0x3A,0x02,   /* !._8..........:. */
 0x1F,0x00,0x3A,0x1E,0x3A,0x1B,0x00,0x00,0x27,0x57,0x7B,0x60,0x00,0x00,0x05,0x02,   /* ..:.:...'W{`.... */
 0x21,0x0C,0x6F,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0xBF,0x00,0x05,0x01,0x3A,0x02,   /* !.o8..........:. */
 0x1F,0x00,0x3A,0x1E,0x3A,0x1B,0x38,0x15,0x38,0x04,0x03,0x52,0x39,0x15,0x00,0x00,   /* ..:.:.8.8..R9... */
 0x00,0x00,0x00,0x00,0x3A,0x02,0x27,0x5C,0x8A,0x60,0x04,0x00,0x23,0x53,0x01,0x2C,   /* ....:.'\.`..#S., */
 0x00,0x00,0x00,0x00,0x1F,0x10,0x05,0x02,0x21,0x0C,0x8B,0x38,0x23,0x5A,0x99,0x60,   /* ........!..8#Z.` */
 0x00,0x00,0xB8,0x00,0x05,0x01,0x05,0x02,0x21,0x0C,0x93,0x38,0x00,0x00,0x00,0x00,   /* ........!..8.... */
 0x00,0x00,0x0B,0x49,0x05,0x01,0x38,0x02,0x39,0x0A,0xCC,0x34,0x3B,0x15,0xA9,0x30,   /* ...I..8.9..4;..0 */
 0x27,0x51,0x28,0x24,0x02,0x28,0x27,0x1B,0x08,0x28,0x3B,0x10,0x1C,0x4F,0x00,0x00,   /* 'Q($.('..(;..O.. */
 0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x02,0x21,0x0C,0xAB,0x38,0x23,0x58,0x09,0x60,   /* ........!..8#X.` */
 0x22,0x02,0x05,0x10,0x05,0x02,0x21,0x0C,0xB2,0x38,0x23,0x5C,0xB8,0x60,0xB8,0x60,   /* ".....!..8#\.`.` */
 0x23,0x5D,0xBC,0x60,0x00,0x00,0xBC,0x60,0x23,0x59,0x09,0x60,0x22,0x02,0x05,0x10,   /* #].`...`#Y.`"... */
 0x05,0x02,0x21,0x0C,0xC0,0x38,0x23,0x5E,0xC6,0x60,0xC6,0x60,0x23,0x5F,0xCA,0x60,   /* ..!..8#^.`.`#_.` */
 0x00,0x00,0xCA,0x60,0x00,0x00,0x09,0x60,0x05,0x02,0x21,0x0C,0xCC,0x38,0x23,0x58,   /* ...`...`..!..8#X */
 0xED,0x60,0x22,0x02,0x05,0x10,0x05,0x02,0x21,0x0C,0xD3,0x38,0x23,0x5C,0xD9,0x60,   /* .`".....!..8#\.` */
 0xD9,0x60,0x23,0x5D,0xDD,0x60,0x00,0x00,0xDD,0x60,0x23,0x59,0xED,0x60,0x22,0x02,   /* .`#].`...`#Y.`". */
 0x05,0x10,0x05,0x02,0x21,0x0C,0xE1,0x38,0x23,0x5E,0xE7,0x60,0xE7,0x60,0x23,0x5F,   /* ....!..8#^.`.`#_ */
 0xEB,0x60,0x00,0x00,0xEB,0x60,0x00,0x00,0xED,0x60,0x27,0x40,0x00,0x28,0x07,0x10,   /* .`...`...`'@.(.. */
 0x24,0x02,0x06,0x10,0x16,0x00,0x17,0x00,0x05,0x01,0x3C,0x01,0x1F,0x01,0x3D,0x01,   /* $.........<...=. */
 0x00,0x00,0x05,0x02,0x25,0x0C,0xF9,0x38,0x1E,0x00,0x03,0x50,0x3D,0x14,0x00,0x00,   /* ....%..8...P=... */
 0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,0x05,0x39,   /* ............%..9 */
 0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x56,0x03,0x70,0x05,0x01,0x00,0x00,0x00,0x00,   /* ...P=.&V.p...... */
 0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,0x12,0x39,0x1E,0x00,0x03,0x50,0x3D,0x14,   /* ......%..9...P=. */
 0x26,0x55,0x03,0x70,0x05,0x01,0x3D,0x51,0xF2,0x60,0x00,0x00,0x00,0x00,0x05,0x02,   /* &U.p..=Q.`...... */
 0x25,0x0C,0x1F,0x39,0x3D,0x02,0x3D,0x1F,0x3D,0x1B,0x26,0x54,0x03,0x70,0x05,0x01,   /* %..9=.=.=.&T.p.. */
 0x3D,0x50,0x3E,0x4A,0x3D,0x01,0x00,0x00,0x05,0x02,0x25,0x0C,0x2C,0x39,0x00,0x00,   /* =P>J=.....%.,9.. */
 0x1E,0x00,0x03,0x50,0x3D,0x14,0x00,0x00,0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,   /* ...P=........... */
 0x00,0x00,0x05,0x02,0x25,0x0C,0x39,0x39,0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x56,   /* ....%.99...P=.&V */
 0x03,0x70,0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,   /* .p............%. */
 0x46,0x39,0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x55,0x03,0x70,0x05,0x01,0x3D,0x51,   /* F9...P=.&U.p..=Q */
 0x1F,0x48,0x3D,0x51,0x3C,0x14,0x05,0x02,0x25,0x0C,0x53,0x39,0x3D,0x02,0x3D,0x1F,   /* .H=Q<...%.S9=.=. */
 0x3D,0x1B,0x26,0x54,0x03,0x70,0x05,0x01,0x3D,0x50,0x3E,0x4A,0x3D,0x01,0x00,0x00,   /* =.&T.p..=P>J=... */
 0x05,0x02,0x25,0x0C,0x60,0x39,0x00,0x00,0x1E,0x00,0x03,0x50,0x3D,0x14,0x00,0x00,   /* ..%.`9.....P=... */
 0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,0x6D,0x39,   /* ............%.m9 */
 0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x56,0x03,0x70,0x05,0x01,0x00,0x00,0x00,0x00,   /* ...P=.&V.p...... */
 0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,0x7A,0x39,0x1E,0x00,0x03,0x50,0x3D,0x14,   /* ......%.z9...P=. */
 0x26,0x55,0x03,0x70,0x05,0x01,0x3D,0x51,0x1F,0x49,0x3D,0x51,0x3C,0x14,0x05,0x02,   /* &U.p..=Q.I=Q<... */
 0x25,0x0C,0x87,0x39,0x3D,0x02,0x3D,0x1F,0x3D,0x1B,0x26,0x54,0x03,0x70,0x05,0x01,   /* %..9=.=.=.&T.p.. */
 0x3D,0x50,0x3E,0x4A,0x3D,0x01,0x00,0x00,0x05,0x02,0x25,0x0C,0x94,0x39,0x00,0x00,   /* =P>J=.....%..9.. */
 0x1E,0x00,0x03,0x50,0x3D,0x14,0x00,0x00,0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,   /* ...P=........... */
 0x00,0x00,0x05,0x02,0x25,0x0C,0xA1,0x39,0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x56,   /* ....%..9...P=.&V */
 0x03,0x70,0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,   /* .p............%. */
 0xAE,0x39,0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x55,0x03,0x70,0x05,0x01,0x3D,0x51,   /* .9...P=.&U.p..=Q */
 0x1F,0x4A,0x3D,0x51,0x3C,0x14,0x05,0x02,0x25,0x0C,0xBB,0x39,0x3D,0x02,0x3D,0x1F,   /* .J=Q<...%..9=.=. */
 0x3D,0x1B,0x26,0x54,0x03,0x70,0x05,0x01,0x3D,0x50,0x3E,0x4A,0x3D,0x01,0x00,0x00,   /* =.&T.p..=P>J=... */
 0x05,0x02,0x25,0x0C,0xC8,0x39,0x00,0x00,0x1E,0x00,0x03,0x50,0x3D,0x14,0x00,0x00,   /* ..%..9.....P=... */
 0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,0xD5,0x39,   /* ............%..9 */
 0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x56,0x03,0x70,0x05,0x01,0x00,0x00,0x00,0x00,   /* ...P=.&V.p...... */
 0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,0xE2,0x39,0x1E,0x00,0x03,0x50,0x3D,0x14,   /* ......%..9...P=. */
 0x26,0x55,0x03,0x70,0x05,0x01,0x3D,0x51,0x1F,0x4B,0x3D,0x51,0x3C,0x14,0x05,0x02,   /* &U.p..=Q.K=Q<... */
 0x25,0x0C,0xEF,0x39,0x3D,0x02,0x3D,0x1F,0x3D,0x1B,0x26,0x54,0x03,0x70,0x05,0x01,   /* %..9=.=.=.&T.p.. */
 0x3D,0x50,0x3E,0x4A,0x3D,0x01,0x00,0x00,0x05,0x02,0x25,0x0C,0xFC,0x39,0x00,0x00,   /* =P>J=.....%..9.. */
 0x1E,0x00,0x03,0x50,0x3D,0x14,0x00,0x00,0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,   /* ...P=........... */
 0x00,0x00,0x05,0x02,0x25,0x0C,0x09,0x3A,0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x56,   /* ....%..:...P=.&V */
 0x03,0x70,0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,   /* .p............%. */
 0x16,0x3A,0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x55,0x03,0x70,0x05,0x01,0x3D,0x51,   /* .:...P=.&U.p..=Q */
 0x1F,0x4C,0x3D,0x51,0x3C,0x14,0x05,0x02,0x25,0x0C,0x23,0x3A,0x3D,0x02,0x3D,0x1F,   /* .L=Q<...%.#:=.=. */
 0x3D,0x1B,0x26,0x54,0x03,0x70,0x05,0x01,0x3D,0x50,0x3E,0x4A,0x3D,0x01,0x00,0x00,   /* =.&T.p..=P>J=... */
 0x05,0x02,0x25,0x0C,0x30,0x3A,0x1E,0x00,0x03,0x50,0x3D,0x14,0x27,0x55,0xD4,0x62,   /* ..%.0:...P=.'U.b */
 0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,0x3D,0x3A,   /* ............%.=: */
 0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x56,0x03,0x70,0x05,0x01,0x00,0x00,0x00,0x00,   /* ...P=.&V.p...... */
 0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,0x4A,0x3A,0x1E,0x00,0x03,0x50,0x3D,0x14,   /* ......%.J:...P=. */
 0x26,0x55,0x03,0x70,0x05,0x01,0x3D,0x51,0x1F,0x4D,0x3D,0x51,0x3C,0x14,0x05,0x02,   /* &U.p..=Q.M=Q<... */
 0x25,0x0C,0x57,0x3A,0x3D,0x02,0x3D,0x1F,0x3D,0x1B,0x26,0x54,0x03,0x70,0x05,0x01,   /* %.W:=.=.=.&T.p.. */
 0x3D,0x50,0x3E,0x4A,0x3D,0x01,0x00,0x00,0x05,0x02,0x25,0x0C,0x64,0x3A,0x1E,0x00,   /* =P>J=.....%.d:.. */
 0x03,0x50,0x3D,0x14,0x27,0x56,0xD4,0x62,0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,   /* .P=.'V.b........ */
 0x00,0x00,0x05,0x02,0x25,0x0C,0x71,0x3A,0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x56,   /* ....%.q:...P=.&V */
 0x03,0x70,0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,   /* .p............%. */
 0x7E,0x3A,0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x55,0x03,0x70,0x05,0x01,0x3D,0x51,   /* ~:...P=.&U.p..=Q */
 0x1F,0x4E,0x3D,0x51,0x3C,0x14,0x05,0x02,0x25,0x0C,0x8B,0x3A,0x3D,0x02,0x3D,0x1F,   /* .N=Q<...%..:=.=. */
 0x3D,0x1B,0x26,0x54,0x03,0x70,0x05,0x01,0x3D,0x50,0x3E,0x4A,0x3D,0x01,0x00,0x00,   /* =.&T.p..=P>J=... */
 0x05,0x02,0x25,0x0C,0x98,0x3A,0x1E,0x00,0x03,0x50,0x3D,0x14,0x27,0x57,0xD4,0x62,   /* ..%..:...P=.'W.b */
 0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,0xA5,0x3A,   /* ............%..: */
 0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x56,0x03,0x70,0x05,0x01,0x00,0x00,0x00,0x00,   /* ...P=.&V.p...... */
 0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,0xB2,0x3A,0x1E,0x00,0x03,0x50,0x3D,0x14,   /* ......%..:...P=. */
 0x26,0x55,0x03,0x70,0x05,0x01,0x3D,0x51,0x1F,0x4F,0x3D,0x51,0x3C,0x14,0x05,0x02,   /* &U.p..=Q.O=Q<... */
 0x25,0x0C,0xBF,0x3A,0x3D,0x02,0x3D,0x1F,0x3D,0x1B,0x26,0x54,0x03,0x70,0x05,0x01,   /* %..:=.=.=.&T.p.. */
 0x3D,0x50,0x3E,0x4A,0x3D,0x01,0x00,0x00,0x05,0x02,0x25,0x0C,0xCC,0x3A,0x00,0x00,   /* =P>J=.....%..:.. */
 0x1E,0x00,0x03,0x50,0x3D,0x14,0x00,0x00,0x05,0x01,0x26,0x59,0x0C,0x63,0x00,0x00,   /* ...P=.....&Y.c.. */
 0x00,0x00,0x05,0x02,0x25,0x0C,0xD9,0x3A,0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x56,   /* ....%..:...P=.&V */
 0x03,0x70,0x05,0x01,0x27,0x54,0x3C,0x01,0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,   /* .p..'T<.......%. */
 0xE6,0x3A,0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x55,0x03,0x70,0x05,0x01,0x3D,0x51,   /* .:...P=.&U.p..=Q */
 0x3C,0x14,0x26,0x02,0x3C,0x1B,0x05,0x02,0x25,0x0C,0xF3,0x3A,0x3D,0x02,0x3D,0x1F,   /* <.&.<...%..:=.=. */
 0x3D,0x1B,0x26,0x54,0x03,0x70,0x05,0x01,0x3D,0x50,0x3E,0x4A,0x3D,0x01,0x00,0x00,   /* =.&T.p..=P>J=... */
 0x05,0x02,0x25,0x0C,0x00,0x3B,0x00,0x00,0x1E,0x00,0x03,0x50,0x3D,0x14,0x00,0x00,   /* ..%..;.....P=... */
 0x05,0x01,0x3C,0x50,0x3E,0x48,0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,0x0D,0x3B,   /* ..<P>H......%..; */
 0x1E,0x00,0x03,0x50,0x3D,0x14,0x26,0x56,0x03,0x70,0x05,0x01,0x26,0x57,0x1D,0x63,   /* ...P=.&V.p..&W.c */
 0x00,0x00,0x00,0x00,0x05,0x02,0x25,0x0C,0x1A,0x3B,0x1E,0x00,0x03,0x50,0x3D,0x14,   /* ......%..;...P=. */
 0x3D,0x59,0x3E,0x49,0x3D,0x02,0x3D,0x1F,0x3D,0x1B,0x3D,0x50,0x3E,0x4A,0x1F,0x02,   /* =Y>I=.=.=.=P>J.. */
 0x3F,0x10,0x1C,0x4F,0xF2,0x60,0x00,0x00,0x00,0x00,0x0B,0x49,0x0B,0x48,0x02,0x23,   /* ?..O.`.....I.H.# */
 0x14,0x00,0x08,0x28,0x3B,0x10,0x3E,0x01,0x28,0x24,0x27,0x50,0x05,0x60,0xED,0x60,   /* ...(;.>.($'P.`.` */
 0x2B,0x63};    /* +c */


u8 pbuf1[] = {
    0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C,
    0x0D, 0x0E, 0x00,
    0x01, 0x02, 0x03,
    0x04, 0x05, 0x06,
    0x55,
    0x01, 0x02, 0x03,
    0x04, 0x05, 0x06,
    0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C,
    0x0D, 0x0E, 0x00,
    0x55,
};

/*********************************************************************
 * @fn      GPIO_Toggle_INIT
 *
 * @brief   Initializes GPIOA.0
 *
 * @return  none
 */
void GPIO_Toggle_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*********************************************************************
 * @fn      PIOC_INIT
 *
 * @brief   Initializes PIOC
 *
 * @return  none
 */
void PIOC_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_19;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_18;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    NVIC_EnableIRQ( PIOC_IRQn );                                        //enable PIOC interrupt
    NVIC_SetPriority(PIOC_IRQn,0xf0);

    memcpy((uint8_t *)(PIOC_SRAM_BASE),PIOC_CODE,sizeof(PIOC_CODE));    // load code for PIOC
    R8_SYS_CFG |= RB_MST_RESET;                                         // reset PIOC
    R8_SYS_CFG = RB_MST_IO_EN1 | RB_MST_IO_EN0;                         // enable IO0&IO1
    R8_SYS_CFG |= RB_MST_CLK_GATE;                                      // open PIOC clock

}


/*********************************************************************
 * @fn      PIOC_IRQHandler
 *
 * @brief   This function handles PIOC exception.
 *
 * @return  none
 */
void PIOC_IRQHandler( void )
{   
    if((R8_SYS_CFG&RB_INT_REQ)!=RESET)
    {
		//Data reception mode
        if((R8_DATA_REG7 & 0X01) == 0)
        {
            rx_buf[rx++] = R8_DATA_REG31;
        }
        //Data Transmission Mode
        //Double buffering with each buffer size of 8 bytes
        else
        {
            if(PIOC_TX_FLAG==1)
            {
                if(PIOC_TX_RemainLEN<=8)
                {
                    memcpy( UART_SFR_ADDR1, PIOC_TX_ADDR, PIOC_TX_RemainLEN );
                    PIOC_TX_FLAG=0;
                }
                else
                {
                    memcpy( UART_SFR_ADDR1, PIOC_TX_ADDR, 8 );
                    PIOC_TX_ADDR += 8;
                    PIOC_TX_RemainLEN -= 8;
                    PIOC_TX_FLAG=2;
                }
            }
            else if(PIOC_TX_FLAG==2)
            {
                if(PIOC_TX_RemainLEN<=8)
                {
                    memcpy( UART_SFR_ADDR2, PIOC_TX_ADDR, PIOC_TX_RemainLEN );
                    PIOC_TX_FLAG=0;
                }
                else
                {
                    memcpy( UART_SFR_ADDR2, PIOC_TX_ADDR, 8 );
                    PIOC_TX_ADDR += 8;
                    PIOC_TX_RemainLEN -= 8;
                    PIOC_TX_FLAG=1;
                }
            }
        }
        R8_CTRL_RD=11;    // False write any value to R8_CTRL_RD, clear interrupt flag
    }
}

/*********************************************************************
 * @fn      PIOC_UART_INIT
 *
 * @brief   Initializes PIOC_UART.
 *
 * @param   baudrate - 921600,460800,230400,115200,57600,38400,
 *                     19200,14400,9600,4800,2400,1200,600,300.
 *          parity - 0: None; 1: Odd; 2: Even; 3: Mark; 4: Space.
 *          stopbits - 0: 1 stop bit; 1: 1.5 stop bit; 2: 2 stop bits
 *          lenth - Data bit length:5,6,7,8
 *
 * @return  none
 */
void PIOC_UART_INIT(uint32_t baudrate,uint8_t parity,uint8_t stopbits,uint8_t lenth)
{
    R8_SYS_CFG |= RB_MST_RESET;                                         // reset PIOC
    R8_SYS_CFG = RB_MST_IO_EN1 | RB_MST_IO_EN0;                         // enable IO0&IO1
    R8_SYS_CFG |= RB_MST_CLK_GATE;                                      // open PIOC clock
    switch(baudrate)
    {
        case 921600: R32_DATA_REG0_3 = 0X5008F525; R32_DATA_REG4_7 = 0X0080FF25; break;
        case 460800: R32_DATA_REG0_3 = 0XB00FE825; R32_DATA_REG4_7 = 0X0010FC25; break;
        case 230400: R32_DATA_REG0_3 = 0XE01CCE25; R32_DATA_REG4_7 = 0X0070F625; break;
        case 115200: R32_DATA_REG0_3 = 0XE0369A25; R32_DATA_REG4_7 = 0X0070E925; break;
        case  57600: R32_DATA_REG0_3 = 0XA06A3225; R32_DATA_REG4_7 = 0X0070CF25; break;
        case  38400: R32_DATA_REG0_3 = 0XF04F6524; R32_DATA_REG4_7 = 0X0070B525; break;
        case  19200: R32_DATA_REG0_3 = 0X704F6423; R32_DATA_REG4_7 = 0X00306725; break;
        case  14400: R32_DATA_REG0_3 = 0X001ACC22; R32_DATA_REG4_7 = 0X00303325; break;
        case   9600: R32_DATA_REG0_3 = 0X0027B222; R32_DATA_REG4_7 = 0X00006524; break;
        case   4800: R32_DATA_REG0_3 = 0X004E6422; R32_DATA_REG4_7 = 0X00006423; break;
        case   2400: R32_DATA_REG0_3 = 0X0027B221; R32_DATA_REG4_7 = 0X0000B222; break;
        case   1200: R32_DATA_REG0_3 = 0X004E6421; R32_DATA_REG4_7 = 0X00006422; break;
        case    600: R32_DATA_REG0_3 = 0X0027B220; R32_DATA_REG4_7 = 0X0000B221; break;
        case    300: R32_DATA_REG0_3 = 0X004E6420; R32_DATA_REG4_7 = 0X00006421; break;
        default:break;
    }
    switch(parity)
    {
        case 1: R8_DATA_REG3 |= 0X0C; R8_DATA_REG6 |= 0X03; break;
        case 2: R8_DATA_REG3 |= 0X04; R8_DATA_REG6 |= 0X02; break;
        case 3: R8_DATA_REG3 |= 0X0C; R8_DATA_REG6 |= 0X03; R8_DATA_REG7 |= 0X10; break;
        case 4: R8_DATA_REG3 |= 0X04; R8_DATA_REG6 |= 0X02; R8_DATA_REG7 |= 0X10; break;
        default:break;
    }
    switch(stopbits)
    {
        case 1: R8_DATA_REG3 |= 0X01; break;
        case 2: R8_DATA_REG3 |= 0X03; break;
        default:break;
    }
    switch(lenth)
    {
        case 5: R8_DATA_REG7 |= 0X20; break;
        case 6: R8_DATA_REG7 |= 0X40; break;
        case 7: R8_DATA_REG7 |= 0X80; break;
        default:break;
    }
}

/*********************************************************************
 * @fn      PIOC_UART_SEND
 *
 * @brief   Copy data to SFR,with a maximum of 16 bytes at a time,
 *          and the remaining bytes are copied during interrupts.
 *
 * @param   p_source_addr - data.
 *          total_bytes - total data number(byte).
 *
 * @return  none
 */
void PIOC_UART_SEND(uint8_t *p_source_addr,uint16_t total_bytes)
{
    PIOC_UART_INIT(PIOC_baudrate,PIOC_parity,PIOC_stopbits,PIOC_word_lenth);
    if(total_bytes<=16)
    {
        memcpy( UART_SFR_ADDR1, p_source_addr, total_bytes );
        PIOC_TX_FLAG = 0;
        PIOC_TX_RemainLEN = 0;
    }
    else
    {
        memcpy( UART_SFR_ADDR1, p_source_addr, 16 );
        PIOC_TX_FLAG = 1;
        PIOC_TX_RemainLEN = total_bytes-16;
        PIOC_TX_ADDR = p_source_addr+16;
    }
    R16_DATA_REG24_25 = total_bytes;
    R8_DATA_REG7 |= 0X01;           //If the 0th bit of register R8_DATA_REG7 is 1, it indicates the sending mode
    R8_CTRL_WR = 0X33;              // To R8_CTRL_WR write any value,then start
}
/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    int i=0;
    int j=0;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(921600);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    printf("GPIO Toggle TEST\r\n");
    GPIO_Toggle_INIT();
    PIOC_INIT();
    PIOC_UART_INIT(PIOC_baudrate,PIOC_parity,PIOC_stopbits,PIOC_word_lenth);
    R8_CTRL_WR = 0X33;                  // To R8_CTRL_WR write any value,then start
    PIOC_UART_SEND(pbuf1,sizeof(pbuf1));
    Delay_Ms(1000);

    while(1)
    {
        if(rx>=30)
        {
            for(j=0;j<30;j++)
            {
                printf("RX DATA:%x\r\n",rx_buf[j]);
            }
            printf("RX STATR:%x\r\n",R8_DATA_REG30);    //0X01: PE  0X02: FE  0X04: NE
            R8_DATA_REG30=0;
            rx=0;
        }
        Delay_Ms(500);
        GPIO_WriteBit(GPIOA, GPIO_Pin_0, (i == 0) ? (i = Bit_SET) : (i = Bit_RESET));
    }
}
