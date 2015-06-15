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

u8 empty_spd[256];
u8 *spd1 = empty_spd, *spd2 = empty_spd;

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
	printk (BIOS_SPEW, "Invalid read %x: %x\n", device, address);
	return -1;
}

int
main (int argc, const char *argv[])
{
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

#include "initial-mchbar.c"

	/* DIMM1 */
	if (argc > 1) {
		int fd = open (argv[1], O_RDONLY);
		if (fd == -1) {
			perror (argv[1]);
			return 1;
		}
		spd1 = mmap (NULL, 256, PROT_READ, MAP_PRIVATE, fd, 0);
		if (spd1 == MAP_FAILED) {
			perror ("mmap SPD1");
			return 1;
		}
		close(fd);
	}

	/* DIMM2 */
	if (argc > 2) {
		int fd = open (argv[2], O_RDONLY);
		if (fd == -1) {
			perror (argv[2]);
			return 1;
		}
		spd2 = mmap (NULL, 256, PROT_READ, MAP_PRIVATE, fd, 0);
		if (spd2 == MAP_FAILED) {
			perror ("mmap SPD2");
			return 1;
		}
		close(fd);
	}

	memset(&sysinfo, 0, sizeof(sysinfo));
	sysinfo.spd_map[0] = 0x50;
	sysinfo.spd_map[2] = 0x51;
	raminit(&sysinfo, 0);


printk (BIOS_SPEW, "=== The End. ===\n");

}
