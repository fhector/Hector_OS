/* bootpack */

#include "bootpack.h"
#include <stdio.h>

#define MEMMAN_FREES		4090		/*approximately 32MB*/
#define MEMMAN_ADDR		0x003c0000

struct FREEINFO{						/*available information*/
	unsigned int addr,size;
};

struct MEMMAN{						/*memory managerment*/
	int frees,maxfrees,lostsize,losts;
	struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC initial finish, and so CPU sti*/
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* set PIC1& keyboard(11111001) */
	io_out8(PIC1_IMR, 0xef); /* set mouse(11101111) */

	init_keyboard();
	enable_mouse(&mdec);
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2; /* compute center point */
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
					/* Ok,Display */
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					/* mouse move */
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* mouse close */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16) {
						my = binfo->scrny - 16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* coordinate fade  */
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* coordinate display */
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* display mouse */
				}
			}
		}
	}
}

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	/* confirm CPU is 386 or 486 + */
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) { /* fi 386, even if set AC=1, AC value will still be 0  */
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* Prohibit the cache */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /*allow the cache  */
		store_cr0(cr0);
	}

	return i;
}

void  memman_init(struct MEMMAN * man)
{
	man->frees = 0;				/*Number of available information*/
	man->maxfrees = 0;			/*Available for observation:frees max value*/
	man->lostsize = 0;			/*The total size of the release of memory failure*/
	man->losts = 0;				/*count of release failure*/
	return;
}

unsigned int memman_total(struct MEMMAN * man)
	/*Report to spare the memory size of the total*/
{
	unsigned int i,t = 0;
	for(i = 0; i < man->frees; i++){
		t += man->free[i].size;
		}
	return t;
}

unsigned int memman_alloc(struct MEMMAN * man,unsigned int size)
	/*allocate*/
{
	unsigned int i,a;
	for (i = 0; i <man->frees;i++){
		if(man->free[i].size >= size){
			/*find */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if(man->free[i].size == 0){
				/*if free[i] == 0, delete it*/
				man->frees--;
				for(; i< man->frees;i++){
					man->free[i] = man->free[i+1]; 
					}
				}
			return a;
			}
		}
	return 0;		/*no free mem*/
}

int memman_free(struct MEMMAN * man,unsigned int addr,unsigned int size)
	/*free*/
{
	int i,j;
	/*In order to facilitate memory, free[] order by addr*/
	/*so, first decided to put it where*/
	for(i = 0; i<man->frees; i++){
		if(man->free[i].addr > addr) {
			break;
			}
		}
	/*free[i-1].addr < addr < free[i].addr*/
	if( i > 0){
		/*prev has available mem*/
		if(man->free[i -1].addr + man->free[i -1].size == addr){
			/*merge with fronter*/
			man->free[i-1].size += size;
			if (i < man->frees){
				/*rear has available mem*/
				if (addr + size == man->free[i].addr){
					/*also merge with rear*/
					man->free[i-1].size+=man->free[i].size;
					/*man->free[i] delete*/
					/*free[i] merged*/
					man->frees--;
					for(;i < man->frees; i++){
						man->free[i] = man->free[i+1]; /**/
						}
					}
				}
			return 0; /*finish*/
			}
		}
	/*not merge the prev*/
	if(i < man->frees){
		/*rear has available mem*/
		if ( addr + size == man->free[i].addr){
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;	
			}
		}
	/* not merge both prev,rear*/
	if(man->frees < MEMMAN_FREES){
		/*after the free[i] should Backward for available space*/
		for(j = man->frees; j>i; j--){
			man->free[j] = man-> free[j-1];
			}
		man-> frees++;
		if(man->maxfrees < man->frees){
			man->maxfrees = man->frees; /*update maxfrees*/
			}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;		/*finish*/
		}
	man->losts++;
	man->lostsize+=size;
	return -1;	/*fail*/
}