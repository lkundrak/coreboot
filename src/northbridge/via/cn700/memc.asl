/* The DRAM controller */
Device (MEMC)
{
	Name (_ADR, 0x00000003)

	OperationRegion (MEMB, PCI_Config, 0x00, 0xEF)
	Field (MEMB, DWordAcc, NoLock, Preserve) {

		/* DRAM Rank Ending Address */
		Offset (0x40),
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
