#include <smp/node.h>
#include <bootblock_common.h>

#include "serial.c"
#include "console.c"

static void main(unsigned long bist)
{
	if (boot_cpu()) {
		bootblock_mainboard_init();

#if CONFIG_USE_OPTION_TABLE
		sanitize_cmos();
#endif
#if CONFIG_CMOS_POST
		cmos_post_init();
#endif
	}

	outb(0xc0, 0x80);
	init_console ();
	outb(0xd0, 0x80);
	sio_init();
	outb(0xe0, 0x80);
	sio_putstring("KONSKY\nKROKOT");
//	outb(0x0e, 0xcf9);

	const char* target1 = "fallback/romstage";
	unsigned long entry;
	entry = findstage(target1);
	if (entry) call(entry, bist);
	asm volatile ("1:\n\thlt\n\tjmp 1b\n\t");
}
