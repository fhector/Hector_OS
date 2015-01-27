

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

void inthandler27(int *esp)
/* 								*/
{
	io_out8(PIC0_OCW2, 0x67); /*  */
	return;
}
