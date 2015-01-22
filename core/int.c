

#include "bootpack.h"
#include <stdio.h>

void init_pic(void)
/* PIC‚initial */
{
	io_out8(PIC0_IMR,  0xff  ); /* ?????? */
	io_out8(PIC1_IMR,  0xff  ); /* ??????*/

	io_out8(PIC0_ICW1, 0x11  ); /* ?????? edge trigger mode */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7?INT20-27?? */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1?IRQ2??*/
	io_out8(PIC0_ICW4, 0x01  ); /* ?????? */

	io_out8(PIC1_ICW1, 0x11  ); /* ?????? edge trigger mode */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15?INT28-2F?? */
	io_out8(PIC1_ICW3, 2     ); /* PIC1?IRQ2??*/
	io_out8(PIC1_ICW4, 0x01  ); /* ?????? */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1?????? */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 ?????? */

	return;
}

#define PORT_KEYDAT		0x0060

struct FIFO8 keyfifo;

void inthandler21(int *esp)
/* PS/2 keyboad interrupt handler */
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	/* IRQ-01 receive irq and go on  */
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);
	return;
}

struct FIFO8 mousefifo;

void inthandler2c(int *esp)
/* PS/2 mouse intrrupt handler */
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);	/* IRQ-12 inform PIC1  */
	io_out8(PIC0_OCW2, 0x62);	/* IRQ-02 then (must)inform PIC0 otherwise ignore */
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
	return;
}

void inthandler27(int *esp)
/* 								*/
{
	io_out8(PIC0_OCW2, 0x67); /*  */
	return;
}
