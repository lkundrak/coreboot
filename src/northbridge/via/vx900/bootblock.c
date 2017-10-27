/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2017 Lubomir Rintel <lkundrak@v3.sk>
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

#include <arch/io.h>

static void bootblock_northbridge_init(void) {
{
        //if (CONFIG_ROM_SIZE == 0x200000)
	pci_io_write_config8(PCI_DEV(0, 0x11, 0), 0x40, 0x04);
	pci_io_write_config8(PCI_DEV(0, 0x11, 0), 0x41, 0xc0);
}
