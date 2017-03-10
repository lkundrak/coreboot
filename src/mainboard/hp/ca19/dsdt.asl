#include <arch/ioapic.h>

DefinitionBlock(
	"dsdt.aml",
	"DSDT",
	0x02,		// DSDT revision: ACPI v1.0
	"COREv4",	// OEM id
	"COREBOOT",	// OEM table id
	0x20170227	// OEM revision
)
{
	#include <arch/x86/acpi/debug.asl>

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
	Name (PICF, Zero)
	Method (_PIC, 1) {
		Store (Arg0, \PICF)
		if (PICF) {
			Store ("=== Selected APIC mode ===", Debug);
			Return ();
		}
		Store ("=== Selected PIC mode ===", Debug);
	}

	Scope (\_SB) {
		Device (LNKA) {
			Name (_HID, EisaId ("PNP0C0F"))
			Name (_UID, 1)

			Name (PRSA, ResourceTemplate () { IRQ (Level, ActiveLow, Shared) {11} })
			Method (_PRS, 0) {
				Store ("=== Called LNKA _PRS ===", Debug)
				Return (PRSA)
			}

			Name (CRSA, ResourceTemplate () { IRQ (Level, ActiveLow, Shared) {11} })
			Method (_CRS, 0) {
				Store ("=== Called LNKA _CRS ===", Debug)
				Return (CRSA)
			}

			Method (_SRS, 1) {
				Store ("=== Called LNKA _SRS ===", Debug)
				Store (Arg0, Debug)
				Store ("=== End of LNKA _SRS ===", Debug)
			}

			Method (_DIS, 0) {
				Store ("=== Called LNKA _DIS ===", Debug)
			}
		}

		Device (LNKB) {
			Name (_HID, EisaId ("PNP0C0F"))
			Name (_UID, 2)

			Name (PRSB, ResourceTemplate () { IRQ (Level, ActiveLow, Shared) {5} })
			Method (_PRS, 0) {
				Store ("=== Called LNKB _PRS ===", Debug)
				Return (PRSB)
			}

			Name (CRSB, ResourceTemplate () { IRQ (Level, ActiveLow, Shared) {5} })
			Method (_CRS, 0) {
				Store ("=== Called LNKB _CRS ===", Debug)
				Return (CRSB)
			}

			Method (_SRS, 1) {
				Store ("=== Called LNKB _SRS ===", Debug)
				Store (Arg0, Debug)
				Store ("=== End of LNKB _SRS ===", Debug)
			}

			Method (_DIS, 0) {
				Store ("=== Called LNKB _DIS ===", Debug)
			}
		}

		Device (LNKC) {
			Name (_HID, EisaId ("PNP0C0F"))
			Name (_UID, 3)

			Name (PRSC, ResourceTemplate () { IRQ (Level, ActiveLow, Shared) {10} })
			Method (_PRS, 0) {
				Store ("=== Called LNKC _PRS ===", Debug)
				Return (PRSC)
			}

			Name (CRSC, ResourceTemplate () { IRQ (Level, ActiveLow, Shared) {10} })
			Method (_CRS, 0) {
				Store ("=== Called LNKC _CRS ===", Debug)
				Return (CRSC)
			}

			Method (_SRS, 1) {
				Store ("=== Called LNKC _SRS ===", Debug)
				Store (Arg0, Debug)
				Store ("=== End of LNKC _SRS ===", Debug)
			}

			Method (_DIS, 0) {
				Store ("=== Called LNKC _DIS ===", Debug)
			}
		}

		Device (LNKD) {
			Name (_HID, EisaId ("PNP0C0F"))
			Name (_UID, 3)

			Name (PRSD, ResourceTemplate () { IRQ (Level, ActiveLow, Shared) {0} })
			Method (_PRS, 0) {
				Store ("=== Called LNKD _PRS ===", Debug)
				Return (PRSD)
			}

			Name (CRSD, ResourceTemplate () { IRQ (Level, ActiveLow, Shared) {0} })
			Method (_CRS, 0) {
				Store ("=== Called LNKD _CRS ===", Debug)
				Return (CRSD)
			}

			Method (_SRS, 1) {
				Store ("=== Called LNKD _SRS ===", Debug)
				Store (Arg0, Debug)
				Store ("=== End of LNKD _SRS ===", Debug)
			}

			Method (_DIS, 0) {
				Store ("=== Called LNKD _DIS ===", Debug)
			}
		}

		/* PCI bus */
		Device (PCI0) {
			Name (_HID, EisaId ("PNP0A03"))
			Name (_UID, 1)
			Name (_ADR, 0x00000000)
			///Name (_BBN, 0)

			Name (XCRS, ResourceTemplate () {
				/* All PCI busses */
				WordBusNumber (ResourceConsumer, MinNotFixed, MaxNotFixed, PosDecode,
					0x0000, 0x0000, 0x00ff, 0x0000, 0x0100,,,)

				/* IO-space, sans the PCI regs. */
				WordIO (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
					0x0000, 0x0000, 0x0CF7, 0x0000, 0x0CF8,
					,,, TypeStatic)
				WordIO (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
					0x0000, 0x0D00, 0xFFFF, 0x0000, 0xF300,
					,,, TypeStatic)

				/* The space from top of DRAM to IOAPIC */
				DWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
					/* This is a template that gets filled in _CRS() */
					0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,,,
					MEM0, AddressRangeMemory, TypeStatic)
			})
			Method (_CRS, 0) {
				Store ("=== PCI0._CRS Called ===", Debug)

				/* MEM0 is from the top of RAM to IOAPIC */
				CreateDWordField (XCRS, \_SB.PCI0.MEM0._MIN, MEML)
				CreateDWordField (XCRS, \_SB.PCI0.MEM0._MAX, MEMH)
				CreateDWordField (XCRS, \_SB.PCI0.MEM0._LEN, LENM)
				Store (\_SB.PCI0.MEMC.TOLM, MEML)
				Subtract (IO_APIC_ADDR, 1, MEMH)
				Subtract (IO_APIC_ADDR, MEML, LENM)

				Return (XCRS);
			}





			/* "Table 9. APIC Fixed IRQ Routing
			 * When the internal APIC is enabled, internal IRQ
			 * routing to the APIC is fixed as follows:" */
			Name (APRT, Package () {
				/* The graphics controller behind the AGP bridge */
				/* INTA# => IRQ16 INTB# => IRQ17 INTC# => IRQ18 INTD# => IRQ19 */
				Package (4) { 0x0001ffff, 0, 0x00, 16 },
				Package (4) { 0x0001ffff, 1, 0x00, 17 },
				Package (4) { 0x0001ffff, 2, 0x00, 18 },
				Package (4) { 0x0001ffff, 3, 0x00, 19 },

				/* IDE (Native Mode)/SATA IRQ & INTE => IRQ20 */
				Package (4) { 0x000fffff, 0, 0x00, 20 },
				Package (4) { 0x000fffff, 1, 0x00, 20 },
				Package (4) { 0x000fffff, 2, 0x00, 20 },
				Package (4) { 0x000fffff, 3, 0x00, 20 },

				/* USB IRQ (all 5 functions) and INTF => IRQ21 */
				Package (4) { 0x0010ffff, 0, 0x00, 21 },
				Package (4) { 0x0010ffff, 1, 0x00, 21 },
				Package (4) { 0x0010ffff, 2, 0x00, 21 },
				Package (4) { 0x0010ffff, 3, 0x00, 21 },

				/* AC’97 / MC’97 IRQ and INTG => IRQ22 */
				Package (4) { 0x0011ffff, 0, 0x00, 22 },
				Package (4) { 0x0011ffff, 1, 0x00, 22 },
				Package (4) { 0x0011ffff, 2, 0x00, 22 },
				Package (4) { 0x0011ffff, 3, 0x00, 22 },

				/* LAN IRQ and INTH => IRQ23 */
				Package (4) { 0x0012ffff, 0, 0x00, 23 },
				Package (4) { 0x0012ffff, 1, 0x00, 23 },
				Package (4) { 0x0012ffff, 2, 0x00, 23 },
				Package (4) { 0x0012ffff, 3, 0x00, 23 }
			})



			Name (XPRT, Package (20) {
				/* AGP bridge */
				Package (4) { 0x0001ffff, 0, LNKA, 0x00 },
				Package (4) { 0x0001ffff, 1, LNKB, 0x00 },
				Package (4) { 0x0001ffff, 2, LNKC, 0x00 },
				Package (4) { 0x0001ffff, 3, LNKD, 0x00 },

				/* IDE interface */
				Package (4) { 0x000fffff, 0, LNKA, 0x00 },
				Package (4) { 0x000fffff, 1, LNKB, 0x00 },
				Package (4) { 0x000fffff, 2, LNKC, 0x00 },
				Package (4) { 0x000fffff, 3, LNKD, 0x00 },

				/* USB controller */
				Package (4) { 0x0010ffff, 0, LNKA, 0x00 },
				Package (4) { 0x0010ffff, 1, LNKB, 0x00 },
				Package (4) { 0x0010ffff, 2, LNKC, 0x00 },
				Package (4) { 0x0010ffff, 3, LNKD, 0x00 },

				/* Audio (& LPC bridge) */
				Package (4) { 0x0011ffff, 0, LNKA, 0x00 },
				Package (4) { 0x0011ffff, 1, LNKB, 0x00 },
				Package (4) { 0x0011ffff, 2, LNKC, 0x00 },
				Package (4) { 0x0011ffff, 3, LNKD, 0x00 },

				/* Ethernet controller */
				Package (4) { 0x0012ffff, 0, LNKA, 0x00 },
				Package (4) { 0x0012ffff, 1, LNKB, 0x00 },
				Package (4) { 0x0012ffff, 2, LNKC, 0x00 },
				Package (4) { 0x0012ffff, 3, LNKD, 0x00 }
			})
			Method (_PRT, 0) {
				Store ("=== Called _PRT ===", Debug)
				If (LEqual (PICF, Zero)) {
					Store ("=== Called _PRT PIC ===", Debug)
					Return (XPRT);
				}
				Store ("=== Called _PRT APIC ===", Debug)
				Return (APRT);
			}

			/* The DRAM controller */
			Device (MEMC)
			{
				Name (_ADR, 0x00000003)

				OperationRegion (MEMB, PCI_Config, 0x00, 0xEF)
				Field (MEMB, DWordAcc, NoLock, Preserve) {
					Offset (0x40),	/* DRAM Rank Ending Address */
					R0EA, 8,	/* Rank 0 Ending Address */
					R1EA, 8,	/* Rank 1 Ending Address */
					R2EA, 8,	/* Rank 2 Ending Address */
					R3EA, 8,	/* Rank 3 Ending Address */
				}

				/* Find the top of DRAM */
				Method (TOLM, 0) {
					/* Find the last occupied rank's end. */
					Store (R3EA, Local0)
					If (LEqual (Local0, Zero)) {
						Store (R2EA, Local0)
					}
					If (LEqual (Local0, Zero)) {
						Store (R1EA, Local0)
					}
					If (LEqual (Local0, Zero)) {
						Store (R0EA, Local0)
					}
					/* The granularity is 64M */
					ShiftLeft (Local0, 26, Local0)
					Return (Local0)
				}
			}
		}
	}
}
