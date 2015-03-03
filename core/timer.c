/* timer.c */

#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC		1	/* configured status */
#define TIMER_FLAGS_USING		2	/* timer operating */

void init_pit(void)
{
	int i;
	struct TIMER *t;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = 0; /* unused */
	}
	t = timer_alloc(); 
	t->timeout = 0xffffffff;
	t->flags = TIMER_FLAGS_USING;
	t->next = 0; /* the end */
	timerctl.t0 = t; /* Because now only the sentinel, so it is in the front */
	timerctl.next = 0xffffffff; /* Because now only the sentinel, so the next is just sentinel */
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	return 0; /* not find */
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0; /* unused */
	return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	t = timerctl.t0;
	if (timer->timeout <= t->timeout) {
		/* Insert the front */
		timerctl.t0 = timer;
		timer->next = t; /* next is set t */
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	/* Search the insert position */
	for (;;) {
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout) {
			/* insert between s and t */
			s->next = timer; 
			timer->next = t; 
			io_store_eflags(e);
			return;
		}
	}
}

void inthandler20(int *esp)
{
	struct TIMER *timer;
	io_out8(PIC0_OCW2, 0x60);	/* IRQ-00 inform GIC */
	timerctl.count++;
	if (timerctl.next > timerctl.count) {
		return;
	}
	timer = timerctl.t0; /* First turn on the front of addresses assigned to the timer */
	for (;;) {	
		if (timer->timeout > timerctl.count) {
			break;
		}
		/* timeout */
		timer->flags = TIMER_FLAGS_ALLOC;
		fifo32_put(timer->fifo, timer->data);
		timer = timer->next; 
	}
	timerctl.t0 = timer;
	timerctl.next = timer->timeout;
	return;
}
