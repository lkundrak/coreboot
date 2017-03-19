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

DefinitionBlock(
	"dsdt.aml",
	"DSDT",
	0x02,		// DSDT revision: ACPI v2.0
	"COREv4",	// OEM id
	"COREBOOT",	// OEM table id
	0x20170227	// OEM revision
)
{
	/* Sleep states */
	Name (\_S0, Package (0x04) { 0x00, 0x00, 0x00, 0x00 })
	Name (\_S5, Package (0x04) { 0x02, 0x02, 0x02, 0x02 })

	/* Interrupts */
	Scope (\_GPE) {
		Method (_L03, 0, NotSerialized) { Store ("=== L03 (LAN) ===", Debug) }
		Method (_L09, 0, NotSerialized) { Store ("=== L09 (USB0,1,2) ===", Debug) }
		Method (_L0E, 0, NotSerialized) { Store ("=== L0E (USB3,3,4,5,6,7) ===", Debug) }
		Method (_L0A, 0, NotSerialized) { Store ("=== L0A (Thermal) ===", Debug) }
		Method (_L05, 0, NotSerialized) { Store ("=== L05 (PCI) ===", Debug) }
		Method (_L08, 0, NotSerialized) { Store ("=== L08 (UART) ===", Debug) }
		Method (_L0D, 0, NotSerialized) { Store ("=== L0D (SOUND) ===", Debug) }
	}

	/* Interrupt model */
	Method (_PIC, 1) {
		Store (Arg0, \_SB.PCI0.ISAC.APIC)
		if (Arg0) {
			Store ("=== Selected APIC mode ===", Debug);
			Return ();
		}
		Store ("=== Selected PIC mode ===", Debug);
	}

	Scope (\_SB) {
		/* PCI bus */
		Device (PCI0) {
			#include <northbridge/via/cn700/acpi/hostbridge.asl>
			#include <southbridge/via/vt8237r/acpi/lpc.asl>
			#include <southbridge/via/vt8237r/acpi/default_irq_route.asl>
		}
	}
}
