
#include "bootpack.h"

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

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}
