/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2012 secunet Security Networks AG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

// __PRE_RAM__ means: use "unsigned" for device, not a struct.

#include <stdint.h>
#include <string.h>
#include <arch/io.h>
#include <cpu/x86/lapic.h>
#include <cpu/x86/msr.h>
#include <cpu/x86/tsc.h>
#include <cbmem.h>
#include <lib.h>
#include <pc80/mc146818rtc.h>
#include <console/console.h>
#include "southbridge/intel/i82801hx/i82801hx.h"
#include "northbridge/intel/i965/i965.h"
#include "northbridge/intel/i965/raminit.h"

#include <device/pnp_def.h>
#include "dock.h"

#define LPC_DEV PCI_DEV(0, 0x1f, 0)
#define MCH_DEV PCI_DEV(0, 0, 0)

static void default_southbridge_gpio_setup(void)
{
#if 0
	outl(0x197e23fe, DEFAULT_GPIOBASE + 0x00);
	outl(0xe1a66dfe, DEFAULT_GPIOBASE + 0x04);
	outl(0xe3faef3f, DEFAULT_GPIOBASE + 0x0c);

	/* Disable blink [31:0]. */
	outl(0x00000000, DEFAULT_GPIOBASE + 0x18);
	/* Set input inversion [31:0]. */
	outl(0x00000102, DEFAULT_GPIOBASE + 0x2c);

	/* Enable GPIOs [60:32]. */
	outl(0x030306f6, DEFAULT_GPIOBASE + 0x30);
	/* Set input/output mode [60:32] (0 == out, 1 == in). */
	outl(0x1f55f9f1, DEFAULT_GPIOBASE + 0x34);
	/* Set gpio levels [60:32].  */
	outl(0x1dffff53, DEFAULT_GPIOBASE + 0x38);
#endif
#if 0 /* x60 */
	printk(BIOS_DEBUG, " GPIOS...");

	/* X60 GPIO:
	 * 1: HDD_PRESENCE#
	 * 6: Unknown (Pulled high by R215 to VCC3B)
	 * 7: BDC_PRESENCE#
	 * 8: H8_WAKE#
	 * 9: RTC_BAT_IN#
	 * 10: Unknown (Pulled high by R700 to VCC3M)
	 * 12: H8SCI#
	 * 13: SLICE_ON_3M#
	 * 14: Unknown (Pulled high by R321 to VCC3)
	 * 15: Unknown (Pulled high by R258 to VCC3)
	 * 19: Unknown (Pulled low  by R594)
	 * 21: Unknown (Pulled high by R145 to VCC3)
	 * 22: FWH_WP#
	 * 25: MDC_KILL#
	 * 33: HDD_PRESENCE_2#
	 * 35: CLKREQ_SATA#
	 * 36: PLANARID0
	 * 37: PLANARID1
	 * 38: PLANARID2
	 * 39: PLANARID3
	 * 48: FWH_TBL#
	 */

	outl(0x1f40f7c2, DEFAULT_GPIOBASE + 0x00);	/* GPIO_USE_SEL */
	outl(0xe0e8ffc3, DEFAULT_GPIOBASE + 0x04);	/* GP_IO_SEL */
	outl(0xfbf6ddfd, DEFAULT_GPIOBASE + 0x0c);	/* GP_LVL */
	/* Output Control Registers */
	outl(0x00040000, DEFAULT_GPIOBASE + 0x18);	/* GPO_BLINK */
	/* Input Control Registers */
	outl(0x000039ff, DEFAULT_GPIOBASE + 0x2c);	/* GPI_INV */
	outl(0x000100f2, DEFAULT_GPIOBASE + 0x30);	/* GPIO_USE_SEL2 */
	outl(0x000000f0, DEFAULT_GPIOBASE + 0x34);	/* GP_IO_SEL2 */
	outl(0x00030043, DEFAULT_GPIOBASE + 0x38);	/* GP_LVL */
#endif

}

static void early_lpc_setup(void)
{
	/* Set up SuperIO LPC forwards */

	/* Configure serial IRQs.*/
	pci_write_config8(LPC_DEV, D31F0_SERIRQ_CNTL, 0xd0);
	/* Map COMa on 0x3f8, COMb on 0x2f8. */
	pci_write_config16(LPC_DEV, D31F0_LPC_IODEC, 0x0010);		// XXX diff
	pci_write_config16(LPC_DEV, D31F0_LPC_EN, 0x3f0f);		// XXX diff
	/* range 0x1600 - 0x167f */
	pci_write_config32(LPC_DEV, D31F0_GEN1_DEC, 0x7c1601);
	/* range 0x15e0 - 0x10ef */
	pci_write_config32(LPC_DEV, D31F0_GEN2_DEC, 0xc15e1);
	/* range 0x1680 - 0x169f */
	pci_write_config32(LPC_DEV, D31F0_GEN3_DEC, 0x1c1681);

#if 0
	// Enable Serial IRQ
	pci_write_config8(PCI_DEV(0, 0x1f, 0), 0x64, 0xd0);
	// decode range
	pci_write_config16(PCI_DEV(0, 0x1f, 0), 0x80, 0x0210);
	// decode range
	pci_write_config16(PCI_DEV(0, 0x1f, 0), 0x82, 0x1f0d);

	/* range 0x1600 - 0x167f */
	pci_write_config16(PCI_DEV(0, 0x1f, 0), 0x84, 0x1601);
	pci_write_config16(PCI_DEV(0, 0x1f, 0), 0x86, 0x007c);

	/* range 0x15e0 - 0x10ef */
	pci_write_config16(PCI_DEV(0, 0x1f, 0), 0x88, 0x15e1);
	pci_write_config16(PCI_DEV(0, 0x1f, 0), 0x8a, 0x000c);

	/* range 0x1680 - 0x169f */
	pci_write_config16(PCI_DEV(0, 0x1f, 0), 0x8c, 0x1681);
	pci_write_config16(PCI_DEV(0, 0x1f, 0), 0x8e, 0x001c);
#endif
}

static void rcba_config(void)
{
#if 0
	int i, size = 0x4000;
	for (i = 0; i < size; i += 4) {
		if (RCBA32(i) + RCBA32(i + 4) + RCBA32(i + 8) + RCBA32(i + 12) == 0)
			continue;
		printk(BIOS_SPEW, "0x%04x: ", i);
		printk(BIOS_SPEW, "0x%08x ", RCBA32(i)); i += 4;
		printk(BIOS_SPEW, "0x%08x ", RCBA32(i)); i += 4;
		printk(BIOS_SPEW, "0x%08x ", RCBA32(i)); i += 4;
		printk(BIOS_SPEW, "0x%08x\n", RCBA32(i));
	}
while (1) printk (BIOS_SPEW, "a");
#else
	die("lala\n");
#endif
#if 0 /* inteltool -a */
	/* Enable ethernet.  */
	RCBA32(0x3414) &= ~0x20; // 3414–3414h BUC Backed Up Control 00h R/W	// wut
		// no reboot flag

	RCBA32(0x0238) = 0x00543210; // 0238–023Bh RPFN Root Port Function Number for PCI Express Root Ports 00543210h R/WO, RO

// wtf
	RCBA32(0x0240) = 0x009c0b02;
	RCBA32(0x0244) = 0x00a20b1a;
	RCBA32(0x0248) = 0x005402cb;
	RCBA32(0x0254) = 0x00470966;
	RCBA32(0x0258) = 0x00470473;
	RCBA32(0x0260) = 0x00e90825;
	RCBA32(0x0278) = 0x00bc0efb;
	RCBA32(0x027c) = 0x00c00f0b;
	RCBA32(0x0280) = 0x00670000;
	RCBA32(0x0284) = 0x006d0000;
	RCBA32(0x0288) = 0x00600b4e;

	RCBA32(0x1e10) = 0x00020800; // 1E10–1E17h TRCR Trapped Cycle Register 0000000000000000h RO
	RCBA32(0x1e18) = 0x36ea00a0; // 1E18-1E1Fh TWDR Trapped Write Data Register 0000000000000000h RO
	RCBA32(0x1e80) = 0x000c0801; // 1E80-1E87h IOTR0 I/O Trap Register 0 0000000000000000h R/W
	RCBA32(0x1e84) = 0x000200f0; // 

//wtf
	RCBA32(0x2028) = 0x04c8f95e;
	RCBA32(0x202c) = 0x055c095e;
	RCBA32(0x204c) = 0x001ffc00;
	RCBA32(0x2050) = 0x00100fff;
	RCBA32(0x2090) = 0x37000000;
	RCBA32(0x20b0) = 0x0c000000;
	RCBA32(0x20d0) = 0x09000000;
	RCBA32(0x20f0) = 0x05000000;

	RCBA32(0x3400) = 0x0000001c; // 3400–3403h RC RTC Configuration 00000000h R/W, R/WLO
	RCBA32(0x3410) = 0x00100461; // 3410–3413h GCS General Control and Status 000000yy0h R/W, R/WLO
	RCBA32(0x3414) = 0x00000000; // 3414–3414h BUC Backed Up Control 00h R/W
	RCBA32(0x341c) = 0xbf4f001f; // 341C–341Fh CG Clock Gating 00000000h R/W
	RCBA32(0x3420) = 0x00000000; // 3420–3420h PDSW 00h R/W
	RCBA32(0x3430) = 0x00000001; // 3430-3433h CIR8 Chipset Initialization Register 8 00000000h R/W
#endif
}

static void early_superio_config(void)
{
	int timeout = 100000;
	device_t dev = PNP_DEV(0x2e, 3);

	pnp_write_config(dev, 0x29, 0x06);

	while (!(pnp_read_config(dev, 0x29) & 0x08) && timeout--)
		udelay(1000);

	/* Enable COM1 */
	pnp_set_logical_device(dev);
	pnp_set_iobase(dev, PNP_IDX_IO0, 0x3f8);
	pnp_set_enable(dev, 1);
}

void main(unsigned long bist)
{
//	sysinfo_t sysinfo;
	int boot_mode = 0;
	int cbmem_initted;
	u16 reg16;
	const u8 spd_addrmap[2 * DIMM_SOCKETS] = { 0x50, 0x52, 0x51, 0x53 };

#if 0 /* nic take ako 0x1c.3 neni? */
	/* Enable expresscard hotplug events.  */
	pci_write_config32(PCI_DEV (0, 0x1c, 3),
			   0xd8,
			   pci_read_config32(PCI_DEV (0, 0x1c, 3), 0xd8)
			   | (1 << 30));
	pci_write_config16(PCI_DEV (0, 0x1c, 3),
			   0x42, 0x141);
#endif

	/* basic northbridge setup, including MMCONF BAR */
	i965_early_init();

	if (bist == 0)
		enable_lapic();

	/* First, run everything needed for console output. */
	i82801hx_early_init();
	early_lpc_setup();

        dlpc_init();
        /* dock_init initializes the DLPC switch on
         *  thinpad side, so this is required even
         *  if we're undocked. 
         */
        if (dock_present()) {
                dock_connect();
                early_superio_config();
        }

	console_init();
	printk(BIOS_DEBUG, "running main(bist = %lu)\n", bist);

//dump_pci_devices();

	reg16 = pci_read_config16(LPC_DEV, D31F0_GEN_PMCON_3);
	pci_write_config16(LPC_DEV, D31F0_GEN_PMCON_3, reg16);
	if ((MCHBAR16(SSKPD_MCHBAR) == 0xCAFE) && !(reg16 & (1 << 9))) {
		printk(BIOS_DEBUG, "soft reset detected, rebooting properly\n");
		i965_early_reset();
	}

	default_southbridge_gpio_setup();

	/* ASPM related setting, set early by original BIOS. */
	DMIBAR16(0x204) &= ~(3 << 10);

	/* Check for S3 resume. */
	const u32 pm1_cnt = inl(DEFAULT_PMBASE + 0x04);
	if (((pm1_cnt >> 10) & 7) == 5) {
#if CONFIG_HAVE_ACPI_RESUME
		printk(BIOS_DEBUG, "Resume from S3 detected.\n");
		boot_mode = 2;
		/* Clear SLP_TYPE. This will break stage2 but
		 * we care for that when we get there.
		 */
		outl(pm1_cnt & ~(7 << 10), DEFAULT_PMBASE + 0x04);
#else
		printk(BIOS_DEBUG, "Resume from S3 detected, but disabled.\n");
#endif
	}

        enable_smbus();
        dump_spd_registers();

        sdram_initialize(boot_mode, spd_addrmap);

#if 0
	/* RAM initialization */
	enter_raminit_or_reset();
	memset(&sysinfo, 0, sizeof(sysinfo));
	sysinfo.spd_map[0] = 0x50;
	sysinfo.spd_map[2] = 0x51;
	sysinfo.enable_igd = 1;
	sysinfo.enable_peg = 0;
	get_gmch_info(&sysinfo);
	raminit(&sysinfo, boot_mode == 2);
#endif

	const u32 deven = pci_read_config32(MCH_DEV, D0F0_DEVEN);
	/* Disable D4F0 (unknown signal controller). */
	pci_write_config32(MCH_DEV, D0F0_DEVEN, deven & ~0x4000);

//	init_pm(&sysinfo, 0);

	i82801hx_dmi_setup();
//	i965_late_init(sysinfo.stepping);
	i965_late_init(0);
	i82801hx_dmi_poll_vc1();

	MCHBAR16(SSKPD_MCHBAR) = 0xCAFE;
	rcba_config ();
	die("NO RAMINIT");

#if 0 /* no iommu */
	init_iommu();
#endif

#if 0
	/* FIXME: make a proper SMBUS mux support.  */
	outl(inl(DEFAULT_GPIOBASE + 0x38) & ~0x400, DEFAULT_GPIOBASE + 0x38);
#endif

	cbmem_initted = !cbmem_recovery(boot_mode == 2);
#if CONFIG_HAVE_ACPI_RESUME
	/* If there is no high memory area, we didn't boot before, so
	 * this is not a resume. In that case we just create the cbmem toc.
	 */
	if ((boot_mode == 2) && cbmem_initted) {
		void *resume_backup_memory = cbmem_find(CBMEM_ID_RESUME);

		/* copy 1MB - 64K to high tables ram_base to prevent memory corruption
		 * through stage 2. We could keep stuff like stack and heap in high tables
		 * memory completely, but that's a wonderful clean up task for another
		 * day.
		 */
		if (resume_backup_memory)
			memcpy(resume_backup_memory, (void *)CONFIG_RAMBASE, HIGH_MEMORY_SAVE);

		/* Magic for S3 resume */
		pci_write_config32(PCI_DEV(0, 0, 0), D0F0_SKPD, SKPAD_ACPI_S3_MAGIC);
	} else {
		/* Magic for S3 resume */
		pci_write_config32(PCI_DEV(0, 0, 0), D0F0_SKPD, SKPAD_NORMAL_BOOT_MAGIC);
	}
#endif
	printk(BIOS_SPEW, "exit main()\n");
}

