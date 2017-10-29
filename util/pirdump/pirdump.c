/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2017 Lubomir Rintel <lkundrak@v3.sk>
 *
 * Structure definitions were copied from
 * src/arch/x86/include/arch/pirq_routing.h:
 *
 * Copyright (C) 2012 Alexandru Gagniuc <mr.nuke.me@gmail.com>
 * Copyright (C) 2012 Patrick Georgi <patrick@georgi-clan.de>
 * Copyright (C) 2010 Stefan Reinauer <stepan@coreboot.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PIRQ_SIGNATURE	(('$' << 0) + ('P' << 8) + ('I' << 16) + ('R' << 24))
#define PIRQ_VERSION 0x0100

struct irq_info {
	uint8_t bus, devfn;		/* Bus, device and function */
	struct {
		uint8_t link;		/* IRQ line ID, chipset dependent, 0=not routed */
		uint16_t bitmap;	/* Available IRQs */
	} __attribute__((packed)) irq[4];
	uint8_t slot;			/* Slot number, 0=onboard */
	uint8_t rfu;
} __attribute__((packed));

struct irq_routing_table {
	uint32_t signature;		/* PIRQ_SIGNATURE should be here */
	uint16_t version;		/* PIRQ_VERSION */
	uint16_t size;			/* Table size in bytes */
	uint8_t  rtr_bus, rtr_devfn;	/* Where the interrupt router lies */
	uint16_t exclusive_irqs;	/* IRQs devoted exclusively to PCI usage */
	uint16_t rtr_vendor, rtr_device;/* Vendor/device ID of interrupt router */
	uint32_t miniport_data;
	uint8_t  rfu[11];
	uint8_t  checksum;		/* Modulo 256 checksum must give zero */
	struct irq_info slots[1];
} __attribute__((packed));

static int
dump (struct irq_routing_table *pir)
{
	int slot, irq;

	printf ("Signature: 0x%08x\n", pir->signature);
	printf ("Version: 0x%04x\n", pir->version);
	printf ("Size: 0x%04x\n", pir->size);
	printf ("Interrupt router bus: 0x%02x\n", pir->rtr_bus);
	printf ("Interrupt router device/function: 0x%02x\n", pir->rtr_devfn);
	printf ("IRQs devoted exclusively to PCI usage: 0x%02x\n", pir->exclusive_irqs);
	printf ("Interrupt router vendor: 0x%04x\n", pir->rtr_vendor);
	printf ("Interrupt router device: 0x%04x\n", pir->rtr_device);
	printf ("Miniport data: 0x%08x\n", pir->miniport_data);
	printf ("Checksum: 0x%02x\n", pir->checksum);

	for (slot = 0; (void *)&pir->slots[slot] < (void *)pir + pir->size; slot++) {
		printf ("Slot: %d\n", slot);
		printf ("\tBus: 0x%02x\n", pir->slots[slot].bus);
		printf ("\tDevice/function: 0x%02x\n", pir->slots[slot].devfn);

		for (irq = 0; irq < 4; irq++) {
			printf ("\tIRQ: %d\n", irq);
			printf ("\t\tLink: 0x%02x\n", pir->slots[slot].irq[irq].link);
			printf ("\t\tBitmap: 0x%04x\n", pir->slots[slot].irq[irq].bitmap);
		}

		printf ("\tSlot number: 0x%02x\n", pir->slots[slot].slot);
	}

	return 0;
}

static int
dumpc (struct irq_routing_table *pir)
{
	int i, slot, irq;

	puts ("const struct irq_routing_table table = {");

	putchar ('\t');
	if (pir->signature == PIRQ_SIGNATURE)
		printf ("PIRQ_SIGNATURE");
	else
		printf ("0x%08x", pir->signature);
	printf (",\t\t/* u32 signature */\n");

	putchar ('\t');
	if (pir->version == PIRQ_VERSION)
		printf ("PIRQ_VERSION");
	else
		printf ("0x%08x", pir->signature);
	printf (",\t\t/* u16 version */\n");

	putchar ('\t');
	if ((pir->size - 32) % 16 == 0)
		printf ("32 + 16 * %d", (pir->size - 32) / 16);
	else
		printf ("%d", pir->size);
	printf (",\t\t/* Max. number of devices on the bus */\n");

	printf ("\t0x%02x,\t\t\t/* Interrupt router bus */\n", pir->rtr_bus);
	printf ("\t(0x%02x << 3) | 0x%x,\t/* Interrupt router dev */\n", pir->rtr_devfn >> 3, pir->rtr_devfn & 7);
	printf ("\t0x%x,\t\t\t/* IRQs devoted exclusively for PCI */\n", pir->exclusive_irqs);
	printf ("\t0x%04x,\t\t\t/* Vendor */\n", pir->rtr_vendor);
	printf ("\t0x%04x,\t\t\t/* Device */\n", pir->rtr_device);
	printf ("\t0x%04x,\t\t\t/* Miniport */\n", pir->miniport_data);
	printf ("\t{");
	for (i = 0; i < 11; i++) {
		printf (" %d", pir->rfu[i]);
		if (i < 10)
			printf (",");
	}
	printf (" }, /* u8 rfu[11] */\n");
	printf ("\t0x%02x,\t\t\t/* Checksum (has to be set to some value that\n", pir->checksum);
	puts ("\t\t\t\t * would give 0 after the sum of all bytes");
	puts ("\t\t\t\t * for this structure (including checksum). */");
	puts ("\t{");

	printf ("\t\t/* bus,        dev | fn,   {link, bitmap}, {link, bitmap}, {link, bitmap}, {link, bitmap}, slot, rfu */\n");
	for (slot = 0; (void *)&pir->slots[slot] < (void *)pir + pir->size; slot++) {
		printf ("\t\t{0x%02x, ", pir->slots[slot].bus);
		printf ("(0x%02x << 3) | 0x%x, {", pir->slots[slot].devfn >> 3, pir->slots[slot].devfn & 7);
		for (irq = 0; irq < 4; irq++) {
			printf ("{0x%02x, 0x%04x}", pir->slots[slot].irq[irq].link, pir->slots[slot].irq[irq].bitmap);
			if (irq < 3)
				printf (", ");
		}
		printf ("}, 0x%x, 0x%x},\n", pir->slots[slot].slot, pir->slots[slot].rfu);
	}
	puts ("\t}\n};");

	return 0;
}

static int
dumpr (struct irq_routing_table *pir)
{
	int written = 0;
	int ret;

	do {
		ret = write (STDOUT_FILENO, pir + written, pir->size - written);
		switch (ret) {
		case -1:
			perror ("write");
			/* fallthrough */
		case 0:
			return 1;
		default:
			written += ret;
		}
	} while (written < pir->size);

	return 0;
}

int
main (int argc, char *argv[])
{
	int fd;
	void *mem;
	struct irq_routing_table *pir;
	const char *pathname;
	size_t size, offset, step;
	struct stat statbuf;
	enum { DEFAULT, C, RAW } format = DEFAULT;
	int arg = 1;

	if (argc > arg && argv[arg][0] == '-') {
		if (   strcmp (argv[arg], "--default") == 0
		    || strcmp (argv[arg], "-d") == 0) {
			format = DEFAULT;
		} else if (  strcasecmp (argv[arg], "--c") == 0
		           || strcasecmp (argv[arg], "-c") == 0) {
			format = C;
		} else if (   strcmp (argv[arg], "--raw") == 0
		           || strcmp (argv[arg], "-r") == 0) {
			format = RAW;
		} else if (strcmp (argv[arg], "--") != 0) {
			fprintf (stderr, "Bad argument: %s\n", argv[arg]);
			return 1;
		}
		arg++;
	}

	if (argc > arg) {
		if (stat (argv[arg], &statbuf) == -1) {
			perror (argv[arg]);
			return 1;
		}

		pathname = argv[arg];
		offset = 0;
		size = statbuf.st_size;
		step = 4;
		arg++;
	} else {
		pathname = "/dev/mem";
		offset = 0xf0000;
		size = 0x10000;
		step = 16;
	}

	if (argc > arg) {
		fprintf (stderr, "Extra argument: %s\n", argv[arg]);
		return 1;
	}

	fd = open (pathname, O_RDONLY);
	if (fd == -1) {
		perror (pathname);
		return 1;
	}

	mem = mmap (NULL, size, PROT_READ, MAP_SHARED, fd, offset);
	if (mem == MAP_FAILED) { 
		perror ("mmap");
		return 1;
	}

	for (pir = mem; (void *)pir < mem + size; pir = (void *)pir + step) {
		if (pir->signature == PIRQ_SIGNATURE)
			break;
	}
	if (pir == mem + size) {
		fprintf (stderr, "$PIR signature not found\n");
		return 1;
	}

	switch (format) {
	case DEFAULT:
		return dump (pir);
	case RAW:
		return dumpr (pir);
	case C:
		return dumpc (pir);
	}
}
