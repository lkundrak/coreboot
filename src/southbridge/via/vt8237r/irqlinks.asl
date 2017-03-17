/* Possible PNP IRQs */
Name (PIRQ, ResourceTemplate () {
	IRQ (Level, ActiveLow, Shared) {3, 4, 6, 7, 10, 11, 12}
})

Device (LNKA) {
	Name (_HID, EisaId ("PNP0C0F"))
	Name (_UID, 1)

	Method (_STA, 0) {
		If (LEqual (\_SB.PCI0.ISAC.INTA, 0x00)) {
			Return (STA_DISABLED)
		}
		Return (STA_INVISIBLE)
	}

	Method (_PRS, 0) {
		Return (PIRQ)
	}

	Name (CRSA, ResourceTemplate () {
		IRQ (Level, ActiveLow, Shared, FUHA) {}
	})
	Method (_CRS, 0) {
		CreateWordField (CRSA, 0x1, AINT)
		ShiftLeft (One, \_SB.PCI0.ISAC.INTA, AINT)
		Return (CRSA)
	}

	Method (_SRS, 1) {

		CreateWordField (Arg0, 0x1, ALAL)
		Store (Zero, Local0)
		Store (ALAL, Local1)
		While (LNotEqual (Local1, One)) {
			ShiftRight (Local1, One, Local1)
			Increment (Local0)
		}
		Store (Local0, \_SB.PCI0.ISAC.INTA)

	}

	Method (_DIS, 0) {
		Store (Zero, \_SB.PCI0.ISAC.INTA)
	}
}

Device (LNKB) {
	Name (_HID, EisaId ("PNP0C0F"))
	Name (_UID, 1)

	Method (_STA, 0) {
		If (LEqual (\_SB.PCI0.ISAC.INTB, 0x00)) {
			Return (STA_DISABLED)
		}
		Return (STA_INVISIBLE)
	}

	Method (_PRS, 0) {
		Return (PIRQ)
	}

	Name (CRSA, ResourceTemplate () {
		IRQ (Level, ActiveLow, Shared, FUHA) {}
	})
	Method (_CRS, 0) {
		CreateWordField (CRSA, 0x1, AINT)
		ShiftLeft (One, \_SB.PCI0.ISAC.INTB, AINT)
		Return (CRSA)
	}

	Method (_SRS, 1) {

		CreateWordField (Arg0, 0x1, ALAL)
		Store (Zero, Local0)
		Store (ALAL, Local1)
		While (LNotEqual (Local1, One)) {
			ShiftRight (Local1, One, Local1)
			Increment (Local0)
		}
		Store (Local0, \_SB.PCI0.ISAC.INTB)

	}

	Method (_DIS, 0) {
		Store (Zero, \_SB.PCI0.ISAC.INTB)
	}
}

Device (LNKC) {
	Name (_HID, EisaId ("PNP0C0F"))
	Name (_UID, 1)

	Method (_STA, 0) {
		If (LEqual (\_SB.PCI0.ISAC.INTC, 0x00)) {
			Return (STA_DISABLED)
		}
		Return (STA_INVISIBLE)
	}

	Method (_PRS, 0) {
		Return (PIRQ)
	}

	Name (CRSA, ResourceTemplate () {
		IRQ (Level, ActiveLow, Shared, FUHA) {}
	})
	Method (_CRS, 0) {
		CreateWordField (CRSA, 0x1, AINT)
		ShiftLeft (One, \_SB.PCI0.ISAC.INTC, AINT)
		Return (CRSA)
	}

	Method (_SRS, 1) {

		CreateWordField (Arg0, 0x1, ALAL)
		Store (Zero, Local0)
		Store (ALAL, Local1)
		While (LNotEqual (Local1, One)) {
			ShiftRight (Local1, One, Local1)
			Increment (Local0)
		}
		Store (Local0, \_SB.PCI0.ISAC.INTC)

	}

	Method (_DIS, 0) {
		Store (Zero, \_SB.PCI0.ISAC.INTC)
	}
}

Device (LNKD) {
	Name (_HID, EisaId ("PNP0C0F"))
	Name (_UID, 1)

	Method (_STA, 0) {
		If (LEqual (\_SB.PCI0.ISAC.INTD, 0x00)) {
			Return (STA_DISABLED)
		}
		Return (STA_INVISIBLE)
	}

	Method (_PRS, 0) {
		Return (PIRQ)
	}

	Name (CRSA, ResourceTemplate () {
		IRQ (Level, ActiveLow, Shared, FUHA) {}
	})
	Method (_CRS, 0) {
		CreateWordField (CRSA, 0x1, AINT)
		ShiftLeft (One, \_SB.PCI0.ISAC.INTD, AINT)
		Return (CRSA)
	}

	Method (_SRS, 1) {

		CreateWordField (Arg0, 0x1, ALAL)
		Store (Zero, Local0)
		Store (ALAL, Local1)
		While (LNotEqual (Local1, One)) {
			ShiftRight (Local1, One, Local1)
			Increment (Local0)
		}
		Store (Local0, \_SB.PCI0.ISAC.INTD)

	}

	Method (_DIS, 0) {
		Store (Zero, \_SB.PCI0.ISAC.INTD)
	}
}
