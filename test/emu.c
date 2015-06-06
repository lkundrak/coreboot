#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <console/console.h>
#include <device/device.h>

#include <northbridge/intel/i965/i965.h>
#include <arch/io.h>

u8 *spd1, *spd2;

void perror(const char *s);
void __attribute__((noreturn)) exit (int);

void die(const char *msg)
{
	print_emerg(msg);
	exit (1);
}

int do_printk(int msg_level, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	return vprintf(fmt, ap);
}

void enable_smbus(void)
{
}

void i965_early_reset(void)
{
}

int smbus_read_byte(unsigned device, unsigned address)
{
	if (device == 0x50)
		return spd1[address];
	else if (device == 0x51)
		return spd2[address];
	printk (BIOS_SPEW, "Invalid read %x: %x <<<\n", device, address);
	return -1;
}

int
main (int argc, const char *argv[])
{
	int dimm1, dimm2;
	sysinfo_t sysinfo;

	/* map PCIEXBAR */
	if (mmap ((void *)CONFIG_MMCONF_BASE_ADDRESS, 32 * 4096,
			PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE,
			-1, 0) != (void *)CONFIG_MMCONF_BASE_ADDRESS) {
		perror ("mmap MMCONF_BASE_ADDRESS");
		return 1;
	}

	/* map DEFAULT_MCHBAR */
	if (mmap ((void *)DEFAULT_MCHBAR, 4 * 4096,
			PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE,
			-1, 0) != (void *)DEFAULT_MCHBAR) {
		perror ("mmap DEFAULT_MCHBAR");
		return 1;
	}

	dimm1 = open (argc > 1 ? argv[1] : "test/dimms/6", O_RDONLY);
	spd1 = mmap (NULL, 256, PROT_READ, MAP_PRIVATE, dimm1, 0);
	dimm2 = open (argc > 2 ? argv[2] : "test/dimms/7", O_RDONLY);
	spd2 = mmap (NULL, 256, PROT_READ, MAP_PRIVATE, dimm2, 0);

	memset(&sysinfo, 0, sizeof(sysinfo));
	sysinfo.spd_map[0] = 0x50;
	sysinfo.spd_map[2] = 0x51;
	raminit(&sysinfo, 0);
}
