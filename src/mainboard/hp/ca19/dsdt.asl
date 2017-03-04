DefinitionBlock(
	"dsdt.aml",
	"DSDT",
	0x02,		// DSDT revision: ACPI v1.0
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
}
