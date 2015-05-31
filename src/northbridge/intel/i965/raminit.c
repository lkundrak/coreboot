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

#include <stdint.h>
#include <stdlib.h>
#include <arch/cpu.h>
#include <arch/io.h>
#include <device/pci_def.h>
#include <device/pnp_def.h>
#include <spd.h>
#include <console/console.h>
#include <lib.h>
#include "delay.h"
#include "i965.h"

#if 0
void collect_dimm_config(sysinfo_t *const sysinfo);
#endif

#if 0
static const gmch_gfx_t gmch_gfx_types[][5] = {
/*  MAX_667MHz    MAX_533MHz    MAX_400MHz    MAX_333MHz    MAX_800MHz    */
  { GMCH_UNKNOWN, GMCH_UNKNOWN, GMCH_UNKNOWN, GMCH_UNKNOWN, GMCH_UNKNOWN  },
  { GMCH_GM47,    GMCH_GM45,    GMCH_UNKNOWN, GMCH_UNKNOWN, GMCH_GM49     },
  { GMCH_GE45,    GMCH_GE45,    GMCH_GE45,    GMCH_GE45,    GMCH_GE45     },
  { GMCH_UNKNOWN, GMCH_GL43,	GMCH_GL40,    GMCH_UNKNOWN, GMCH_UNKNOWN  },
  { GMCH_UNKNOWN, GMCH_GS45,    GMCH_GS40,    GMCH_UNKNOWN, GMCH_UNKNOWN  },
  { GMCH_UNKNOWN, GMCH_UNKNOWN, GMCH_UNKNOWN, GMCH_UNKNOWN, GMCH_UNKNOWN  },
  { GMCH_UNKNOWN, GMCH_UNKNOWN, GMCH_UNKNOWN, GMCH_UNKNOWN, GMCH_UNKNOWN  },
  { GMCH_PM45,    GMCH_PM45,    GMCH_PM45,    GMCH_PM45,    GMCH_PM45     },
};

void get_gmch_info(sysinfo_t *sysinfo)
{
	sysinfo->stepping = pci_read_config8(PCI_DEV(0, 0, 0), PCI_CLASS_REVISION);
	if ((sysinfo->stepping > STEPPING_B3) &&
			(sysinfo->stepping != STEPPING_CONVERSION_A1))
	       die("Unknown stepping.\n");
	if (sysinfo->stepping <= STEPPING_B3)
		printk(BIOS_DEBUG, "Stepping %c%d\n", 'A' + sysinfo->stepping / 4, sysinfo->stepping % 4);
	else
		printk(BIOS_DEBUG, "Conversion stepping A1\n");

	const u32 eax = cpuid_ext(0x04, 0).eax;
	sysinfo->cores = ((eax >> 26) & 0x3f) + 1;
	printk(BIOS_SPEW, "%d CPU cores\n", sysinfo->cores);

	u32 capid = pci_read_config16(PCI_DEV(0, 0, 0), D0F0_CAPID0+8);
	if (!(capid & (1<<(79-64)))) {
		printk(BIOS_SPEW, "iTPM enabled\n");
	}

	capid = pci_read_config32(PCI_DEV(0, 0, 0), D0F0_CAPID0+4);
	if (!(capid & (1<<(57-32)))) {
		printk(BIOS_SPEW, "ME enabled\n");
	}

	if (!(capid & (1<<(56-32)))) {
		printk(BIOS_SPEW, "AMT enabled\n");
	}

	sysinfo->max_ddr2_mhz = (capid & (1<<(53-32)))?667:800;
	printk(BIOS_SPEW, "capable of DDR2 of %d MHz or lower\n", sysinfo->max_ddr2_mhz);

	if (!(capid & (1<<(48-32)))) {
		printk(BIOS_SPEW, "VT-d enabled\n");
	}

	const u32 gfx_variant = (capid>>(42-32)) & 0x7;
	const u32 render_freq = ((capid>>(50-32) & 0x1) << 2) | ((capid>>(35-32)) & 0x3);
	if (render_freq <= 4)
		sysinfo->gfx_type = gmch_gfx_types[gfx_variant][render_freq];
	else
		sysinfo->gfx_type = GMCH_UNKNOWN;
	sysinfo->gs45_low_power_mode = 0;
	switch (sysinfo->gfx_type) {
		case GMCH_GM45:
			printk(BIOS_SPEW, "GMCH: GM45\n");
			break;
		case GMCH_GM47:
			printk(BIOS_SPEW, "GMCH: GM47\n");
			break;
		case GMCH_GM49:
			printk(BIOS_SPEW, "GMCH: GM49\n");
			break;
		case GMCH_GE45:
			printk(BIOS_SPEW, "GMCH: GE45\n");
			break;
		case GMCH_GL40:
			printk(BIOS_SPEW, "GMCH: GL40\n");
			break;
		case GMCH_GL43:
			printk(BIOS_SPEW, "GMCH: GL43\n");
			break;
		case GMCH_GS40:
			printk(BIOS_SPEW, "GMCH: GS40\n");
			break;
		case GMCH_GS45:
			printk(BIOS_SPEW, "GMCH: GS45, using low power mode by default\n");
			sysinfo->gs45_low_power_mode = 1;
			break;
		case GMCH_PM45:
			printk(BIOS_SPEW, "GMCH: PM45\n");
			break;
		case GMCH_UNKNOWN:
			printk(BIOS_SPEW, "unknown GMCH\n");
			break;
	}

	sysinfo->txt_enabled = !(capid & (1 << (37-32)));
	if (sysinfo->txt_enabled) {
		printk(BIOS_SPEW, "TXT enabled\n");
	}

	switch (render_freq) {
		case 4:
			sysinfo->max_render_mhz = 800;
			break;
		case 0:
			sysinfo->max_render_mhz = 667;
			break;
		case 1:
			sysinfo->max_render_mhz = 533;
			break;
		case 2:
			sysinfo->max_render_mhz = 400;
			break;
		case 3:
			sysinfo->max_render_mhz = 333;
			break;
		default:
			printk(BIOS_SPEW, "Unknown render frequency\n");
			sysinfo->max_render_mhz = 0;
			break;
	}
	if (sysinfo->max_render_mhz != 0) {
		printk(BIOS_SPEW, "Render frequency: %d MHz\n", sysinfo->max_render_mhz);
	}

	if (!(capid & (1<<(33-32)))) {
		printk(BIOS_SPEW, "IGD enabled\n");
	}

	if (!(capid & (1<<(32-32)))) {
		printk(BIOS_SPEW, "PCIe-to-GMCH enabled\n");
	}

	capid = pci_read_config32(PCI_DEV(0, 0, 0), D0F0_CAPID0);

	u32 ddr_cap = capid>>30 & 0x3;
	switch (ddr_cap) {
		case 0:
			sysinfo->max_ddr3_mt = 1067;
			break;
		case 1:
			sysinfo->max_ddr3_mt = 800;
			break;
		case 2:
		case 3:
			printk(BIOS_SPEW, "GMCH not DDR3 capable\n");
			sysinfo->max_ddr3_mt = 0;
			break;
	}
	if (sysinfo->max_ddr3_mt != 0) {
		printk(BIOS_SPEW, "GMCH supports DDR3 with %d MT or less\n", sysinfo->max_ddr3_mt);
	}

	const unsigned max_fsb = (capid >> 28) & 0x3;
	switch (max_fsb) {
		case 1:
			sysinfo->max_fsb_mhz = 1067;
			break;
		case 2:
			sysinfo->max_fsb_mhz = 800;
			break;
		case 3:
			sysinfo->max_fsb_mhz = 667;
			break;
		default:
			die("unknown FSB capability\n");
			break;
	}
	if (sysinfo->max_fsb_mhz != 0) {
		printk(BIOS_SPEW, "GMCH supports FSB with up to %d MHz\n", sysinfo->max_fsb_mhz);
	}
	sysinfo->max_fsb = max_fsb - 1;
}
#endif

/*
 * Detect if the system went through an interrupted RAM init or is incon-
 * sistent. If so, initiate a cold reboot. Otherwise mark the system to be
 * in RAM init, so this function would detect it on an erreneous reboot.
 */
void enter_raminit_or_reset(void)
{
	/* Interrupted RAM init or inconsistent system? */
	u8 reg8 = pci_read_config8(PCI_DEV(0, 0x1f, 0), 0xa2);

	if (reg8 & (1 << 2)) { /* S4-assertion-width violation */
		/* Ignore S4-assertion-width violation like original BIOS. */
		printk(BIOS_WARNING,
			"WARNING: Ignoring S4-assertion-width violation.\n");
		/* Bit2 is R/WC, so it will clear itself below. */
	}

	if (reg8 & (1 << 7)) { /* interrupted RAM init */
		/* Don't enable S4-assertion stretch. Makes trouble on roda/rk9.
		reg8 = pci_read_config8(PCI_DEV(0, 0x1f, 0), 0xa4);
		pci_write_config8(PCI_DEV(0, 0x1f, 0), 0xa4, reg8 | 0x08);
		*/

		/* Clear bit7. */
		pci_write_config8(PCI_DEV(0, 0x1f, 0), 0xa2, reg8 & ~(1 << 7));

		printk(BIOS_INFO, "Interrupted RAM init, reset required.\n");
		i965_early_reset();
	}
	/* Mark system to be in RAM init. */
	pci_write_config8(PCI_DEV(0, 0x1f, 0), 0xa2, reg8 | (1 << 7));
}


static void reset_on_bad_warmboot(void)
{
	/* Check self refresh channel status. */
	const u32 reg = MCHBAR32(SLFRCS_MCHBAR);
	/* Clear status bits. R/WC */
	MCHBAR32(SLFRCS_MCHBAR) = reg | 1;
	if ((reg & SLFRCS_WARM_RESET) && !(reg & SLFRCS_BOTH_SELFREFRESH)) {
		printk(BIOS_INFO, "DRAM was not in self refresh "
			"during warm boot, reset required.\n");
		i965_early_reset();
	}
}

#if 0
static void set_system_memory_frequency(const timings_t *const timings)
{
	MCHBAR16(CLKCFG_MCHBAR + 0x60) &= ~(1 << 15);
	MCHBAR16(CLKCFG_MCHBAR + 0x48) &= ~(1 << 15);

	/* Calculate wanted frequency setting. */
	const int want_freq = 6 - timings->mem_clock;

	/* Read current memory frequency. */
	const u32 clkcfg = MCHBAR32(CLKCFG_MCHBAR);
	int cur_freq = (clkcfg & CLKCFG_MEMCLK_MASK) >> CLKCFG_MEMCLK_SHIFT;
	if (0 == cur_freq) {
		/* Try memory frequency from scratchpad. */
		printk(BIOS_DEBUG, "Reading current memory frequency from scratchpad.\n");
		cur_freq = (MCHBAR16(SSKPD_MCHBAR) & SSKPD_CLK_MASK) >> SSKPD_CLK_SHIFT;
	}

	if (cur_freq != want_freq) {
		printk(BIOS_DEBUG, "Changing memory frequency: old %x, new %x.\n", cur_freq, want_freq);
		/* When writing new frequency setting, reset, then set update bit. */
		MCHBAR32(CLKCFG_MCHBAR) = (MCHBAR32(CLKCFG_MCHBAR) & ~(CLKCFG_UPDATE | CLKCFG_MEMCLK_MASK)) |
					  (want_freq << CLKCFG_MEMCLK_SHIFT);
		MCHBAR32(CLKCFG_MCHBAR) = (MCHBAR32(CLKCFG_MCHBAR) & ~CLKCFG_MEMCLK_MASK) |
					  (want_freq << CLKCFG_MEMCLK_SHIFT) | CLKCFG_UPDATE;
		/* Reset update bit. */
		MCHBAR32(CLKCFG_MCHBAR) &= ~CLKCFG_UPDATE;
	}

	if ((timings->fsb_clock == FSB_CLOCK_1067MHz) && (timings->mem_clock == MEM_CLOCK_667MT)) {
		MCHBAR32(CLKCFG_MCHBAR + 0x16) = 0x000030f0;
		MCHBAR32(CLKCFG_MCHBAR + 0x64) = 0x000050c1;

		MCHBAR32(CLKCFG_MCHBAR) = (MCHBAR32(CLKCFG_MCHBAR) & ~(1 << 12)) | (1 << 17);
		MCHBAR32(CLKCFG_MCHBAR) |= (1 << 17) | (1 << 12);
		MCHBAR32(CLKCFG_MCHBAR) &= ~(1 << 12);

		MCHBAR32(CLKCFG_MCHBAR + 0x04) = 0x9bad1f1f;
		MCHBAR8(CLKCFG_MCHBAR + 0x08) = 0xf4;
		MCHBAR8(CLKCFG_MCHBAR + 0x0a) = 0x43;
		MCHBAR8(CLKCFG_MCHBAR + 0x0c) = 0x10;
		MCHBAR8(CLKCFG_MCHBAR + 0x0d) = 0x80;
		MCHBAR32(CLKCFG_MCHBAR + 0x50) = 0x0b0e151b;
		MCHBAR8(CLKCFG_MCHBAR + 0x54) = 0xb4;
		MCHBAR8(CLKCFG_MCHBAR + 0x55) = 0x10;
		MCHBAR8(CLKCFG_MCHBAR + 0x56) = 0x08;

		MCHBAR32(CLKCFG_MCHBAR) |= (1 << 10);
		MCHBAR32(CLKCFG_MCHBAR) |= (1 << 11);
		MCHBAR32(CLKCFG_MCHBAR) &= ~(1 << 10);
		MCHBAR32(CLKCFG_MCHBAR) &= ~(1 << 11);
	}

	MCHBAR32(CLKCFG_MCHBAR + 0x48) |= 0x3f << 24;
}
#endif

int raminit_read_vco_index(void)
{
	switch (MCHBAR8(0x0c0f) & 0x7) {
	case VCO_2666:
		return 0;
	case VCO_3200:
		return 1;
	case VCO_4000:
		return 2;
	case VCO_5333:
		return 3;
	default:
		die("Unknown VCO frequency.\n");
		return 0;
	}
}

#if 0
static void set_igd_memory_frequencies(const sysinfo_t *const sysinfo)
{
	const int gfx_idx = ((sysinfo->gfx_type == GMCH_GS45) &&
				!sysinfo->gs45_low_power_mode)
		? (GMCH_GS45 + 1) : sysinfo->gfx_type;

	/* Render and sampler frequency values seem to be some kind of factor. */
	const u16 render_freq_from_vco_and_gfxtype[][10] = {
	/*              GM45  GM47  GM49  GE45  GL40  GL43  GS40  GS45 (perf) */
	/* VCO 2666 */ { 0xd,  0xd,  0xe,  0xd,  0xb,  0xd,  0xb,  0xa,  0xd },
	/* VCO 3200 */ { 0xd,  0xe,  0xf,  0xd,  0xb,  0xd,  0xb,  0x9,  0xd },
	/* VCO 4000 */ { 0xc,  0xd,  0xf,  0xc,  0xa,  0xc,  0xa,  0x9,  0xc },
	/* VCO 5333 */ { 0xb,  0xc,  0xe,  0xb,  0x9,  0xb,  0x9,  0x8,  0xb },
	};
	const u16 sampler_freq_from_vco_and_gfxtype[][10] = {
	/*              GM45  GM47  GM49  GE45  GL40  GL43  GS40  GS45 (perf) */
	/* VCO 2666 */ { 0xc,  0xc,  0xd,  0xc,  0x9,  0xc,  0x9,  0x8,  0xc },
	/* VCO 3200 */ { 0xc,  0xd,  0xe,  0xc,  0x9,  0xc,  0x9,  0x8,  0xc },
	/* VCO 4000 */ { 0xa,  0xc,  0xd,  0xa,  0x8,  0xa,  0x8,  0x8,  0xa },
	/* VCO 5333 */ { 0xa,  0xa,  0xc,  0xa,  0x7,  0xa,  0x7,  0x6,  0xa },
	};
	const u16 display_clock_select_from_gfxtype[] = {
		/* GM45  GM47  GM49  GE45  GL40  GL43  GS40  GS45 (perf) */
		      1,    1,    1,    1,    1,    1,    1,    0,    1
	};

	if (pci_read_config16(GCFGC_PCIDEV, 0) != 0x8086) {
		printk(BIOS_DEBUG, "Skipping IGD memory frequency setting.\n");
		return;
	}

	MCHBAR16(0x119e) = 0xa800;
	MCHBAR16(0x11c0) = (MCHBAR16(0x11c0) & ~0xff00) | (0x01 << 8);
	MCHBAR16(0x119e) = 0xb800;
	MCHBAR8(0x0f10) |= 1 << 7;

	/* Read VCO. */
	const int vco_idx = raminit_read_vco_index();
	printk(BIOS_DEBUG, "Setting IGD memory frequencies for VCO #%d.\n", vco_idx);

	const u32 freqcfg =
		((render_freq_from_vco_and_gfxtype[vco_idx][gfx_idx]
			<< GCFGC_CR_SHIFT) & GCFGC_CR_MASK) |
		((sampler_freq_from_vco_and_gfxtype[vco_idx][gfx_idx]
			<< GCFGC_CS_SHIFT) & GCFGC_CS_MASK);

	/* Set frequencies, clear update bit. */
	u32 gcfgc = pci_read_config16(GCFGC_PCIDEV, GCFGC_OFFSET);
	gcfgc &= ~(GCFGC_CS_MASK | GCFGC_UPDATE | GCFGC_CR_MASK);
	gcfgc |= freqcfg;
	pci_write_config16(GCFGC_PCIDEV, GCFGC_OFFSET, gcfgc);

	/* Set frequencies, set update bit. */
	gcfgc = pci_read_config16(GCFGC_PCIDEV, GCFGC_OFFSET);
	gcfgc &= ~(GCFGC_CS_MASK | GCFGC_CR_MASK);
	gcfgc |= freqcfg | GCFGC_UPDATE;
	pci_write_config16(GCFGC_PCIDEV, GCFGC_OFFSET, gcfgc);

	/* Clear update bit. */
	pci_write_config16(GCFGC_PCIDEV, GCFGC_OFFSET,
		pci_read_config16(GCFGC_PCIDEV, GCFGC_OFFSET) & ~GCFGC_UPDATE);

	/* Set display clock select bit. */
	pci_write_config16(GCFGC_PCIDEV, GCFGC_OFFSET,
		(pci_read_config16(GCFGC_PCIDEV, GCFGC_OFFSET) & ~GCFGC_CD_MASK) |
		(display_clock_select_from_gfxtype[gfx_idx] << GCFGC_CD_SHIFT));
}

static void configure_dram_control_mode(const timings_t *const timings, const dimminfo_t *const dimms)
{
	int ch, r;

	FOR_EACH_CHANNEL(ch) {
		unsigned int mchbar = CxDRC0_MCHBAR(ch);
		u32 cxdrc = MCHBAR32(mchbar);
		cxdrc &= ~CxDRC0_RANKEN_MASK;
		FOR_EACH_POPULATED_RANK_IN_CHANNEL(dimms, ch, r)
			cxdrc |= CxDRC0_RANKEN(r);
		cxdrc = (cxdrc & ~CxDRC0_RMS_MASK) |
				/* Always 7.8us for DDR3: */
				CxDRC0_RMS_78US;
		MCHBAR32(mchbar) = cxdrc;

		mchbar = CxDRC1_MCHBAR(ch);
		cxdrc = MCHBAR32(mchbar);
		cxdrc |= CxDRC1_NOTPOP_MASK;
		FOR_EACH_POPULATED_RANK_IN_CHANNEL(dimms, ch, r)
			cxdrc &= ~CxDRC1_NOTPOP(r);
		cxdrc |= CxDRC1_MUSTWR;
		MCHBAR32(mchbar) = cxdrc;

		mchbar = CxDRC2_MCHBAR(ch);
		cxdrc = MCHBAR32(mchbar);
		cxdrc |= CxDRC2_NOTPOP_MASK;
		FOR_EACH_POPULATED_RANK_IN_CHANNEL(dimms, ch, r)
			cxdrc &= ~CxDRC2_NOTPOP(r);
		cxdrc |= CxDRC2_MUSTWR;
		if (timings->mem_clock == MEM_CLOCK_1067MT)
			cxdrc |= CxDRC2_CLK1067MT;
		MCHBAR32(mchbar) = cxdrc;
	}
}

static void rcomp_initialization(const stepping_t stepping, const int sff)
{
	/* Programm RCOMP codes. */
	if (sff)
		die("SFF platform unsupported in RCOMP initialization.\n");
	/* Values are for DDR3. */
	MCHBAR8(0x6ac) &= ~0x0f;
	MCHBAR8(0x6b0)  =  0x55;
	MCHBAR8(0x6ec) &= ~0x0f;
	MCHBAR8(0x6f0)  =  0x66;
	MCHBAR8(0x72c) &= ~0x0f;
	MCHBAR8(0x730)  =  0x66;
	MCHBAR8(0x76c) &= ~0x0f;
	MCHBAR8(0x770)  =  0x66;
	MCHBAR8(0x7ac) &= ~0x0f;
	MCHBAR8(0x7b0)  =  0x66;
	MCHBAR8(0x7ec) &= ~0x0f;
	MCHBAR8(0x7f0)  =  0x66;
	MCHBAR8(0x86c) &= ~0x0f;
	MCHBAR8(0x870)  =  0x55;
	MCHBAR8(0x8ac) &= ~0x0f;
	MCHBAR8(0x8b0)  =  0x66;
	/* ODT multiplier bits. */
	MCHBAR32(0x04d0) = (MCHBAR32(0x04d0) & ~((7 << 3) | (7 << 0))) | (2 << 3) | (2 << 0);

	/* Perform RCOMP calibration for DDR3. */
	raminit_rcomp_calibration(stepping);

	/* Run initial RCOMP. */
	MCHBAR32(0x418) |= 1 << 17;
	MCHBAR32(0x40c) &= ~(1 << 23);
	MCHBAR32(0x41c) &= ~((1 << 7) | (1 << 3));
	MCHBAR32(0x400) |= 1;
	while (MCHBAR32(0x400) & 1) {}

	/* Run second RCOMP. */
	MCHBAR32(0x40c) |= 1 << 19;
	MCHBAR32(0x400) |= 1;
	while (MCHBAR32(0x400) & 1) {}

	/* Cleanup and start periodic RCOMP. */
	MCHBAR32(0x40c) &= ~(1 << 19);
	MCHBAR32(0x40c) |= 1 << 23;
	MCHBAR32(0x418) &= ~(1 << 17);
	MCHBAR32(0x41c) |= (1 << 7) | (1 << 3);
	MCHBAR32(0x400) |= (1 << 1);
}

static void dram_powerup(const int resume)
{
#if 0
	udelay_from_reset(200);
	MCHBAR32(CLKCFG_MCHBAR) = (MCHBAR32(CLKCFG_MCHBAR) & ~(1 << 3)) | (3 << 21);
	if (!resume) {
		MCHBAR32(0x1434) |= (1 << 10);
		ns100delay(2);
	}
	MCHBAR32(0x1434) |= (1 << 6);
	if (!resume) {
		ns100delay(1);
		MCHBAR32(0x1434) |= (1 << 9);
		MCHBAR32(0x1434) &= ~(1 << 10);
		udelay(500);
	}
#endif
}
static void dram_program_timings(const timings_t *const timings)
{
	/* Values are for DDR3. */
	const int burst_length = 8;
	const int tWTR = 4, tRTP = 1;
	int i;

	FOR_EACH_CHANNEL(i) {
		u32 reg = MCHBAR32(CxDRT0_MCHBAR(i));
		const int btb_wtp = timings->tWL + burst_length/2 + timings->tWR;
		const int btb_wtr = timings->tWL + burst_length/2 + tWTR;
		reg = (reg & ~(CxDRT0_BtB_WtP_MASK  | CxDRT0_BtB_WtR_MASK)) |
			((btb_wtp << CxDRT0_BtB_WtP_SHIFT) & CxDRT0_BtB_WtP_MASK) |
			((btb_wtr << CxDRT0_BtB_WtR_SHIFT) & CxDRT0_BtB_WtR_MASK);
		if (timings->mem_clock != MEM_CLOCK_1067MT) {
			reg = (reg & ~(0x7 << 15)) | ((9 - timings->CAS) << 15);
			reg = (reg & ~(0xf << 10)) | ((timings->CAS - 3) << 10);
		} else {
			reg = (reg & ~(0x7 << 15)) | ((10 - timings->CAS) << 15);
			reg = (reg & ~(0xf << 10)) | ((timings->CAS - 4) << 10);
		}
		reg = (reg & ~(0x7 << 5)) | (3 << 5);
		reg = (reg & ~(0x7 << 0)) | (1 << 0);
		MCHBAR32(CxDRT0_MCHBAR(i)) = reg;

		reg = MCHBAR32(CxDRT1_MCHBAR(i));
		reg = (reg & ~(0x03 << 28)) | ((tRTP & 0x03) << 28);
		reg = (reg & ~(0x1f << 21)) | ((timings->tRAS & 0x1f) << 21);
		reg = (reg & ~(0x07 << 10)) | (((timings->tRRD - 2) & 0x07) << 10);
		reg = (reg & ~(0x07 <<  5)) | (((timings->tRCD - 2) & 0x07) << 5);
		reg = (reg & ~(0x07 <<  0)) | (((timings->tRP - 2) & 0x07) << 0);
		MCHBAR32(CxDRT1_MCHBAR(i)) = reg;

		reg = MCHBAR32(CxDRT2_MCHBAR(i));
		reg = (reg & ~(0x1f << 17)) | ((timings->tFAW & 0x1f) << 17);
		if (timings->mem_clock != MEM_CLOCK_1067MT) {
			reg = (reg & ~(0x7 << 12)) | (0x2 << 12);
			reg = (reg & ~(0xf <<  6)) | (0x9 <<  6);
		} else {
			reg = (reg & ~(0x7 << 12)) | (0x3 << 12);
			reg = (reg & ~(0xf <<  6)) | (0xc <<  6);
		}
		reg = (reg & ~(0x1f << 0)) | (0x13 << 0);
		MCHBAR32(CxDRT2_MCHBAR(i)) = reg;

		reg = MCHBAR32(CxDRT3_MCHBAR(i));
		reg |= 0x3 << 28;
		reg = (reg & ~(0x03 << 26));
		reg = (reg & ~(0x07 << 23)) | (((timings->CAS - 3) & 0x07) << 23);
		reg = (reg & ~(0xff << 13)) | ((timings->tRFC & 0xff) << 13);
		reg = (reg & ~(0x07 <<  0)) | (((timings->tWL - 2) & 0x07) <<  0);
		MCHBAR32(CxDRT3_MCHBAR(i)) = reg;

		reg = MCHBAR32(CxDRT4_MCHBAR(i));
		static const u8 timings_by_clock[4][3] = {
			/*   333MHz  400MHz  533MHz
			     667MT   800MT  1067MT   */
			{     0x07,   0x0a,   0x0d   },
			{     0x3a,   0x46,   0x5d   },
			{     0x0c,   0x0e,   0x18   },
			{     0x21,   0x28,   0x35   },
		};
		const int clk_idx = 2 - timings->mem_clock;
		reg = (reg & ~(0x01f << 27)) | (timings_by_clock[0][clk_idx] << 27);
		reg = (reg & ~(0x3ff << 17)) | (timings_by_clock[1][clk_idx] << 17);
		reg = (reg & ~(0x03f << 10)) | (timings_by_clock[2][clk_idx] << 10);
		reg = (reg & ~(0x1ff <<  0)) | (timings_by_clock[3][clk_idx] <<  0);
		MCHBAR32(CxDRT4_MCHBAR(i)) = reg;

		reg = MCHBAR32(CxDRT5_MCHBAR(i));
		if (timings->mem_clock == MEM_CLOCK_1067MT)
			reg = (reg & ~(0xf << 28)) | (0x8 << 28);
		reg = (reg & ~(0x00f << 22)) | ((burst_length/2 + timings->CAS + 2) << 22);
		reg = (reg & ~(0x3ff << 12)) | (0x190 << 12);
		reg = (reg & ~(0x00f <<  4)) | ((timings->CAS - 2) << 4);
		reg = (reg & ~(0x003 <<  2)) | (0x001 <<  2);
		reg = (reg & ~(0x003 <<  0));
		MCHBAR32(CxDRT5_MCHBAR(i)) = reg;

		reg = MCHBAR32(CxDRT6_MCHBAR(i));
		reg = (reg & ~(0xffff << 16)) | (0x066a << 16); /* always 7.8us refresh rate for DDR3 */
		reg |= (1 <<  2);
		MCHBAR32(CxDRT6_MCHBAR(i)) = reg;
	}
}

static void dram_program_banks(const dimminfo_t *const dimms)
{
	int ch, r;

	FOR_EACH_CHANNEL(ch) {
		const int tRPALL = dimms[ch].banks == 8;

		u32 reg = MCHBAR32(CxDRT1_MCHBAR(ch)) & ~(0x01 << 15);
		IF_CHANNEL_POPULATED(dimms, ch)
			reg |= tRPALL << 15;
		MCHBAR32(CxDRT1_MCHBAR(ch)) = reg;

		reg = MCHBAR32(CxDRA_MCHBAR(ch)) & ~CxDRA_BANKS_MASK;
		FOR_EACH_POPULATED_RANK_IN_CHANNEL(dimms, ch, r) {
			reg |= CxDRA_BANKS(r, dimms[ch].banks);
		}
		MCHBAR32(CxDRA_MCHBAR(ch)) = reg;
	}
}

static void odt_setup(const timings_t *const timings, const int sff)
{
	/* Values are for DDR3. */
	int ch;

	FOR_EACH_CHANNEL(ch) {
		u32 reg = MCHBAR32(CxODT_HIGH(ch));
		if (sff && (timings->mem_clock != MEM_CLOCK_1067MT))
			reg &= ~(0x3 << (61 - 32));
		else
			reg |= 0x3 << (61 - 32);
		reg = (reg & ~(0x3 << (52 - 32))) | (0x2 << (52 - 32));
		reg = (reg & ~(0x7 << (48 - 32))) | ((timings->CAS - 3) << (48 - 32));
		reg = (reg & ~(0xf << (44 - 32))) | (0x7 << (44 - 32));
		if (timings->mem_clock != MEM_CLOCK_1067MT) {
			reg = (reg & ~(0xf << (40 - 32))) | ((12 - timings->CAS) << (40 - 32));
			reg = (reg & ~(0xf << (36 - 32))) | (( 2 + timings->CAS) << (36 - 32));
		} else {
			reg = (reg & ~(0xf << (40 - 32))) | ((13 - timings->CAS) << (40 - 32));
			reg = (reg & ~(0xf << (36 - 32))) | (( 1 + timings->CAS) << (36 - 32));
		}
		reg = (reg & ~(0xf << (32 - 32))) | (0x7 << (32 - 32));
		MCHBAR32(CxODT_HIGH(ch)) = reg;

		reg = MCHBAR32(CxODT_LOW(ch));
		reg = (reg & ~(0x7 << 28)) | (0x2 << 28);
		reg = (reg & ~(0x3 << 22)) | (0x2 << 22);
		reg = (reg & ~(0x7 << 12)) | (0x2 << 12);
		reg = (reg & ~(0x7 <<  4)) | (0x2 <<  4);
		switch (timings->mem_clock) {
		case MEM_CLOCK_667MT:
			reg = (reg & ~0x7);
			break;
		case MEM_CLOCK_800MT:
			reg = (reg & ~0x7) | 0x2;
			break;
		case MEM_CLOCK_1067MT:
			reg = (reg & ~0x7) | 0x5;
			break;
		}
		MCHBAR32(CxODT_LOW(ch)) = reg;
	}
}

static void misc_settings(const timings_t *const timings,
			  const stepping_t stepping)
{
	MCHBAR32(0x1260) = (MCHBAR32(0x1260) & ~((1 << 24) | 0x1f)) | timings->tRD;
	MCHBAR32(0x1360) = (MCHBAR32(0x1360) & ~((1 << 24) | 0x1f)) | timings->tRD;

	MCHBAR8(0x1268) = (MCHBAR8(0x1268) & ~(0xf)) | timings->tWL;
	MCHBAR8(0x1368) = (MCHBAR8(0x1368) & ~(0xf)) | timings->tWL;
	MCHBAR8(0x12a0) = (MCHBAR8(0x12a0) & ~(0xf)) | 0xa;
	MCHBAR8(0x13a0) = (MCHBAR8(0x13a0) & ~(0xf)) | 0xa;

	MCHBAR32(0x218) = (MCHBAR32(0x218) & ~((7 << 29) | (7 << 25) | (3 << 22) | (3 << 10))) |
					       (4 << 29) | (3 << 25) | (0 << 22) | (1 << 10);
	MCHBAR32(0x220) = (MCHBAR32(0x220) & ~(7 << 16)) | (1 << 21) | (1 << 16);
	MCHBAR32(0x224) = (MCHBAR32(0x224) & ~(7 <<  8)) | (3 << 8);
	if (stepping >= STEPPING_B1)
		MCHBAR8(0x234) |= (1 << 3);
}
#endif

static void clock_crossing_setup(const fsb_clock_t fsb,
				 const mem_clock_t mem,
				 const dimminfo_t *const dimms)
{
	int ch;

	static const u32 values_from_fsb_and_mem[][3][4] = {
	/* FSB 800MHz */{
		/* DDR2-667  */ { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff },
		/* DDR2-533  */ { 0xffffffff, 0xffffffff, 0x01c00038, 0x00070e00 },
		},
	/* FSB 667MHz */{
		/* DDR2-667  */ { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff },
		/* DDR2-533  */ { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff },
		},
	/* FSB 533MHz */{
		/* DDR2-667  */ { 0, 0, 0, 0 },
		/* DDR2-533  */ { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff },
		},
	};

	const u32 *data = values_from_fsb_and_mem[fsb][mem];
	MCHBAR32(0x0208) = data[3];
	MCHBAR32(0x020c) = data[2];
#if 0
	if (((fsb == FSB_CLOCK_1067MHz) || (fsb == FSB_CLOCK_800MHz)) && (mem == MEM_CLOCK_667MT))
		MCHBAR32(0x0210) = data[1];
#endif

	static const u32 from_fsb_and_mem[][3] = {
			 /* DDR2-667   DDR2-533 */
	/* FSB  800MHz */{ 0xffffffff, 0x00020108, },
	/* FSB  667MHz */{ 0xffffffff, 0xffffffff, },
	/* FSB  533MHz */{ 0xffffffff, 0xffffffff, },
	};
	FOR_EACH_CHANNEL(ch) {
		const unsigned int mchbar = 0x1258 + (ch * 0x0100);
#if 0
		if ((fsb == FSB_CLOCK_1067MHz) && (mem == MEM_CLOCK_800MT) && CHANNEL_IS_CARDF(dimms, ch))
			MCHBAR32(mchbar) = 0x08040120;
		else
			MCHBAR32(mchbar) = from_fsb_and_mem[fsb][mem];
#else
		MCHBAR32(mchbar) = from_fsb_and_mem[fsb][mem];
#endif
		MCHBAR32(mchbar + 4) = 0x00000000;
	}
}

#if 0
/* Program egress VC1 timings. */
static void vc1_program_timings(const fsb_clock_t fsb)
{
	const u32 timings_by_fsb[][2] = {
	/* FSB 1067MHz */ { 0x1a, 0x01380138 },
	/* FSB  800MHz */ { 0x14, 0x00f000f0 },
	/* FSB  667MHz */ { 0x10, 0x00c000c0 },
	};
	EPBAR8(0x2c)  = timings_by_fsb[fsb][0];
	EPBAR32(0x38) = timings_by_fsb[fsb][1];
	EPBAR32(0x3c) = timings_by_fsb[fsb][1];
}
#endif

/* @prejedec if not zero, set rank size to 128MB and page size to 4KB. */
static void program_memory_map(const dimminfo_t *const dimms, const channel_mode_t mode, const int prejedec, u16 ggc)
{
	int ch, r;

	/* Program rank boundaries (CxDRBy). */
	unsigned int base = 0; /* start of next rank in MB */
	unsigned int total_mb[2] = { 0, 0 }; /* total memory per channel in MB */
	FOR_EACH_CHANNEL(ch) {
		if (mode == CHANNEL_MODE_DUAL_INTERLEAVED)
			/* In interleaved mode, start every channel from 0. */
			base = 0;
		for (r = 0; r < RANKS_PER_CHANNEL; r += 2) {
			/* Fixed capacity for pre-jedec config. */
			const unsigned int rank_capacity_mb =
				prejedec ? 128 : dimms[ch].rank_capacity_mb;
			u32 reg = 0;

			/* Program bounds in CxDRBy. */
			IF_RANK_POPULATED(dimms, ch, r) {
				base += rank_capacity_mb;
				total_mb[ch] += rank_capacity_mb;
			}
			reg |= CxDRBy_BOUND_MB(r, base);
			IF_RANK_POPULATED(dimms, ch, r+1) {
				base += rank_capacity_mb;
				total_mb[ch] += rank_capacity_mb;
			}
			reg |= CxDRBy_BOUND_MB(r+1, base);

			MCHBAR32(CxDRBy_MCHBAR(ch, r)) = reg;
		}
	}

	/* Program page size (CxDRA). */
	FOR_EACH_CHANNEL(ch) {
		u32 reg = MCHBAR32(CxDRA_MCHBAR(ch)) & ~CxDRA_PAGESIZE_MASK;
		FOR_EACH_POPULATED_RANK_IN_CHANNEL(dimms, ch, r) {
			/* Fixed page size for pre-jedec config. */
			const unsigned int page_size = /* dimm page size in bytes */
				prejedec ? 4096 : dimms[ch].page_size;
			reg |= CxDRA_PAGESIZE(r, log2(page_size));
			/* deferred to f5_27: reg |= CxDRA_BANKS(r, dimms[ch].banks); */
		}
		MCHBAR32(CxDRA_MCHBAR(ch)) = reg;
	}

	/* Calculate memory mapping, all values in MB. */

	u32 uma_sizem = 0;
	if (!prejedec) {
		if (!(ggc & 2)) {
			printk(BIOS_DEBUG, "IGD decoded, subtracting ");

			/* Graphics memory */
			const u32 gms_sizek = decode_igd_memory_size((ggc >> 4) & 0xf);
			printk(BIOS_DEBUG, "%uM UMA", gms_sizek >> 10);

			/* GTT Graphics Stolen Memory Size (GGMS) */
			const u32 gsm_sizek = decode_igd_gtt_size((ggc >> 8) & 0xf);
			printk(BIOS_DEBUG, " and %uM GTT\n", gsm_sizek >> 10);

			uma_sizem = (gms_sizek + gsm_sizek) >> 10;
			/* Further reduce MTRR usage if it costs use less than
			   16 MiB.  */
			if (ALIGN_UP(uma_sizem, 64) - uma_sizem <= 16)
				uma_sizem = ALIGN_UP(uma_sizem, 64);
		}
	}

	const unsigned int MMIOstart = 0x0c00 + uma_sizem; /* 3GB, makes MTRR configuration small. */
	const int me_active = pci_read_config8(PCI_DEV(0, 3, 0), PCI_CLASS_REVISION) != 0xff;
	const unsigned int ME_SIZE = prejedec || !me_active ? 0 : 32;
	const unsigned int usedMEsize = (total_mb[0] != total_mb[1]) ? ME_SIZE : 2 * ME_SIZE;
	const unsigned int claimCapable =
		!(pci_read_config32(PCI_DEV(0, 0, 0), D0F0_CAPID0 + 4) & (1 << (47 - 32)));

	const unsigned int TOM = total_mb[0] + total_mb[1];
	unsigned int TOMminusME = TOM - usedMEsize;
	unsigned int TOLUD = (TOMminusME < MMIOstart) ? TOMminusME : MMIOstart;
	unsigned int TOUUD = TOMminusME;
	unsigned int REMAPbase = 0xffff, REMAPlimit = 0;

	if (claimCapable && (TOMminusME >= (MMIOstart + 64))) {
		/* 64MB alignment: We'll lose some MBs here, if ME is on. */
		TOMminusME &= ~(64 - 1);
		/* 64MB alignment: Loss will be reclaimed. */
		TOLUD &= ~(64 - 1);
		if (TOMminusME > 4096) {
			REMAPbase = TOMminusME;
			REMAPlimit = REMAPbase + (4096 - TOLUD);
		} else {
			REMAPbase = 4096;
			REMAPlimit = REMAPbase + (TOMminusME - TOLUD);
		}
		TOUUD = REMAPlimit;
		/* REMAPlimit is an inclusive bound, all others exclusive. */
		REMAPlimit -= 64;
	}

	pci_write_config16(PCI_DEV(0, 0, 0), D0F0_TOM, (TOM >> 7) & 0x1ff);
	pci_write_config16(PCI_DEV(0, 0, 0), D0F0_TOLUD, TOLUD << 4);
	pci_write_config16(PCI_DEV(0, 0, 0), D0F0_TOUUD, TOUUD);
	pci_write_config16(PCI_DEV(0, 0, 0), D0F0_REMAPBASE, (REMAPbase >> 6) & 0x03ff);
	pci_write_config16(PCI_DEV(0, 0, 0), D0F0_REMAPLIMIT, (REMAPlimit >> 6) & 0x03ff);

	/* Program channel mode. */
	switch (mode) {
	case CHANNEL_MODE_SINGLE:
		printk(BIOS_DEBUG, "Memory configured in single-channel mode.\n");
		MCHBAR32(DCC_MCHBAR) &= ~DCC_INTERLEAVED;
		break;
	case CHANNEL_MODE_DUAL_ASYNC:
		printk(BIOS_DEBUG, "Memory configured in dual-channel assymetric mode.\n");
		MCHBAR32(DCC_MCHBAR) &= ~DCC_INTERLEAVED;
		break;
	case CHANNEL_MODE_DUAL_INTERLEAVED:
		printk(BIOS_DEBUG, "Memory configured in dual-channel interleaved mode.\n");
		MCHBAR32(DCC_MCHBAR) &= ~(DCC_NO_CHANXOR | (1 << 9));
		MCHBAR32(DCC_MCHBAR) |= DCC_INTERLEAVED;
		break;
	}

	printk(BIOS_SPEW, "Memory map:\n"
			  "TOM   = %5uMB\n"
			  "TOLUD = %5uMB\n"
			  "TOUUD = %5uMB\n"
			  "REMAP:\t base  = %5uMB\n"
				"\t limit = %5uMB\n"
	                  "usedMEsize: %dMB\n",
			  TOM, TOLUD, TOUUD, REMAPbase, REMAPlimit, usedMEsize);
}

static void prejedec_memory_map(const dimminfo_t *const dimms, channel_mode_t mode)
{
	/* Never use dual-interleaved mode in pre-jedec config. */
	if (CHANNEL_MODE_DUAL_INTERLEAVED == mode)
		mode = CHANNEL_MODE_DUAL_ASYNC;

	program_memory_map(dimms, mode, 1, 0);
	MCHBAR32(DCC_MCHBAR) |= DCC_NO_CHANXOR;
}

#if 0
static void ddr3_select_clock_mux(const mem_clock_t ddr3clock,
				  const dimminfo_t *const dimms,
				  const stepping_t stepping)
{
	const int clk1067 = (ddr3clock == MEM_CLOCK_1067MT);
	const int cardF[] = { CHANNEL_IS_CARDF(dimms, 0), CHANNEL_IS_CARDF(dimms, 1) };

	int ch;

	if (stepping < STEPPING_B1)
		die("Stepping <B1 unsupported in clock-multiplexer selection.\n");

	FOR_EACH_POPULATED_CHANNEL(dimms, ch) {
		int mixed = 0;
		if ((1 == ch) && (!CHANNEL_IS_POPULATED(dimms, 0) || (cardF[0] != cardF[1])))
			mixed = 4 << 11;
		const unsigned int b = 0x14b0 + (ch * 0x0100);
		MCHBAR32(b+0x1c) = (MCHBAR32(b+0x1c) & ~(7 << 11)) |
					(((             cardF[ch])?1:0) << 11) | mixed;
		MCHBAR32(b+0x18) = (MCHBAR32(b+0x18) & ~(7 << 11))             | mixed;
		MCHBAR32(b+0x14) = (MCHBAR32(b+0x14) & ~(7 << 11)) |
					(((!clk1067 && !cardF[ch])?0:1) << 11) | mixed;
		MCHBAR32(b+0x10) = (MCHBAR32(b+0x10) & ~(7 << 11)) |
					((( clk1067 && !cardF[ch])?1:0) << 11) | mixed;
		MCHBAR32(b+0x0c) = (MCHBAR32(b+0x0c) & ~(7 << 11)) |
					(((             cardF[ch])?3:2) << 11) | mixed;
		MCHBAR32(b+0x08) = (MCHBAR32(b+0x08) & ~(7 << 11)) |
					(2 << 11)                              | mixed;
		MCHBAR32(b+0x04) = (MCHBAR32(b+0x04) & ~(7 << 11)) |
					(((!clk1067 && !cardF[ch])?2:3) << 11) | mixed;
		MCHBAR32(b+0x00) = (MCHBAR32(b+0x00) & ~(7 << 11)) |
					((( clk1067 && !cardF[ch])?3:2) << 11) | mixed;
	}
}
static void ddr3_write_io_init(const mem_clock_t ddr3clock,
			       const dimminfo_t *const dimms,
			       const stepping_t stepping,
			       const int sff)
{
	const int a1step = stepping >= STEPPING_CONVERSION_A1;
	const int cardF[] = { CHANNEL_IS_CARDF(dimms, 0), CHANNEL_IS_CARDF(dimms, 1) };

	int ch;

	if (stepping < STEPPING_B1)
		die("Stepping <B1 unsupported in write i/o initialization.\n");
	if (sff)
		die("SFF platform unsupported in write i/o initialization.\n");

	static const u32 ddr3_667_800_by_stepping_ddr3_and_card[][2][2][4] = {
	{ /* Stepping B3 and below */
		{ /* 667 MHz */
			{ 0xa3255008, 0x26888209, 0x26288208, 0x6188040f },
			{ 0x7524240b, 0xa5255608, 0x232b8508, 0x5528040f },
		},
		{ /* 800 MHz */
			{ 0xa6255308, 0x26888209, 0x212b7508, 0x6188040f },
			{ 0x7524240b, 0xa6255708, 0x132b7508, 0x5528040f },
		},
	},
	{ /* Conversion stepping A1 and above */
		{ /* 667 MHz */
			{ 0xc5257208, 0x26888209, 0x26288208, 0x6188040f },
			{ 0x7524240b, 0xc5257608, 0x232b8508, 0x5528040f },
		},
		{ /* 800 MHz */
			{ 0xb6256308, 0x26888209, 0x212b7508, 0x6188040f },
			{ 0x7524240b, 0xb6256708, 0x132b7508, 0x5528040f },
		}
	}};

	static const u32 ddr3_1067_by_channel_and_card[][2][4] = {
		{ /* Channel A */
			{ 0xb2254708, 0x002b7408, 0x132b8008, 0x7228060f },
			{ 0xb0255008, 0xa4254108, 0x4528b409, 0x9428230f },
		},
		{ /* Channel B */
			{ 0xa4254208, 0x022b6108, 0x132b8208, 0x9228210f },
			{ 0x6024140b, 0x92244408, 0x252ba409, 0x9328360c },
		},
	};

	FOR_EACH_POPULATED_CHANNEL(dimms, ch) {
		if ((1 == ch) && CHANNEL_IS_POPULATED(dimms, 0) && (cardF[0] == cardF[1]))
			/* Only write if second channel population differs. */
			continue;
		const u32 *const data = (ddr3clock != MEM_CLOCK_1067MT)
			? ddr3_667_800_by_stepping_ddr3_and_card[a1step][2 - ddr3clock][cardF[ch]]
			: ddr3_1067_by_channel_and_card[ch][cardF[ch]];
		MCHBAR32(CxWRTy_MCHBAR(ch, 0)) = data[0];
		MCHBAR32(CxWRTy_MCHBAR(ch, 1)) = data[1];
		MCHBAR32(CxWRTy_MCHBAR(ch, 2)) = data[2];
		MCHBAR32(CxWRTy_MCHBAR(ch, 3)) = data[3];
	}

	MCHBAR32(0x1490) = 0x00e70067;
	MCHBAR32(0x1494) = 0x000d8000;
	MCHBAR32(0x1590) = 0x00e70067;
	MCHBAR32(0x1594) = 0x000d8000;
}
static void ddr3_read_io_init(const mem_clock_t ddr3clock,
			      const dimminfo_t *const dimms,
			      const int sff)
{
	int ch;

	FOR_EACH_POPULATED_CHANNEL(dimms, ch) {
		u32 addr, tmp;
		const unsigned int base = 0x14b0 + (ch * 0x0100);
		for (addr = base + 0x1c; addr >= base; addr -= 4) {
			tmp = MCHBAR32(addr);
			tmp &= ~((3 << 25) | (1 << 8) | (7 << 16) | (0xf << 20) | (1 << 27));
			tmp |= (1 << 27);
			switch (ddr3clock) {
				case MEM_CLOCK_667MT:
					tmp |= (1 << 16) | (4 << 20);
					break;
				case MEM_CLOCK_800MT:
					tmp |= (2 << 16) | (3 << 20);
					break;
				case MEM_CLOCK_1067MT:
					if (!sff)
						tmp |= (2 << 16) | (1 << 20);
					else
						tmp |= (2 << 16) | (2 << 20);
					break;
				default:
					die("Wrong clock");
			}
			MCHBAR32(addr) = tmp;
		}
	}
}

static void memory_io_init(const mem_clock_t ddr3clock,
			   const dimminfo_t *const dimms,
			   const stepping_t stepping,
			   const int sff)
{
	u32 tmp;

	if (stepping < STEPPING_B1)
		die("Stepping <B1 unsupported in "
			"system-memory i/o initialization.\n");

	tmp = MCHBAR32(0x1400);
	tmp &= ~(3<<13);
	tmp |= (1<<9) | (1<<13);
	MCHBAR32(0x1400) = tmp;

	tmp = MCHBAR32(0x140c);
	tmp &= ~(0xff | (1<<11) | (1<<12) |
		 (1<<16) | (1<<18) | (1<<27) | (0xf<<28));
	tmp |= (1<<7) | (1<<11) | (1<<16);
	switch (ddr3clock) {
		case MEM_CLOCK_667MT:
			tmp |= 9 << 28;
			break;
		case MEM_CLOCK_800MT:
			tmp |= 7 << 28;
			break;
		case MEM_CLOCK_1067MT:
			tmp |= 8 << 28;
			break;
	}
	MCHBAR32(0x140c) = tmp;

	MCHBAR32(0x1440) &= ~1;

	tmp = MCHBAR32(0x1414);
	tmp &= ~((1<<20) | (7<<11) | (0xf << 24) | (0xf << 16));
	tmp |= (3<<11);
	switch (ddr3clock) {
		case MEM_CLOCK_667MT:
			tmp |= (2 << 24) | (10 << 16);
			break;
		case MEM_CLOCK_800MT:
			tmp |= (3 << 24) | (7 << 16);
			break;
		case MEM_CLOCK_1067MT:
			tmp |= (4 << 24) | (4 << 16);
			break;
	}
	MCHBAR32(0x1414) = tmp;

	MCHBAR32(0x1418) &= ~((1<<3) | (1<<11) | (1<<19) | (1<<27));

	MCHBAR32(0x141c) &= ~((1<<3) | (1<<11) | (1<<19) | (1<<27));

	MCHBAR32(0x1428) |= 1<<14;

	tmp = MCHBAR32(0x142c);
	tmp &= ~((0xf << 8) | (0x7 << 20) | 0xf | (0xf << 24));
	tmp |= (0x3 << 20) | (5 << 24);
	switch (ddr3clock) {
		case MEM_CLOCK_667MT:
			tmp |= (2 << 8) | 0xc;
			break;
		case MEM_CLOCK_800MT:
			tmp |= (3 << 8) | 0xa;
			break;
		case MEM_CLOCK_1067MT:
			tmp |= (4 << 8) | 0x7;
			break;
	}
	MCHBAR32(0x142c) = tmp;

	tmp = MCHBAR32(0x400);
	tmp &= ~((3 << 4) | (3 << 16) | (3 << 30));
	tmp |= (2 << 4) | (2 << 16);
	MCHBAR32(0x400) = tmp;

	MCHBAR32(0x404) &= ~(0xf << 20);

	MCHBAR32(0x40c) &= ~(1 << 6);

	tmp = MCHBAR32(0x410);
	tmp &= ~(7 << 28);
	tmp |= 2 << 28;
	MCHBAR32(0x410) = tmp;

	tmp = MCHBAR32(0x41c);
	tmp &= ~0x77;
	tmp |= 0x11;
	MCHBAR32(0x41c) = tmp;

	ddr3_select_clock_mux(ddr3clock, dimms, stepping);

	ddr3_write_io_init(ddr3clock, dimms, stepping, sff);

	ddr3_read_io_init(ddr3clock, dimms, sff);
}
#endif

static void
pre_jedec_settings1 (void)
{
	/* Pre-jedec settings */
	MCHBAR32(0x40) |= (1 << 1);
	MCHBAR32(0x230) |= (3 << 1);
	MCHBAR32(0x238) |= (3 << 24);
	MCHBAR32(0x23c) |= (3 << 24);
}

#if 0
static void jedec_init(const timings_t *const timings,
		       const dimminfo_t *const dimms)
{
	if ((timings->tWR < 5) || (timings->tWR > 12))
		die("tWR value unsupported in Jedec initialization.\n");

	/* Pre-jedec settings */
	MCHBAR32(0x40) |= (1 << 1);
	MCHBAR32(0x230) |= (3 << 1);
	MCHBAR32(0x238) |= (3 << 24);
	MCHBAR32(0x23c) |= (3 << 24);

	/* Normal write pointer operation */
	MCHBAR32(0x14f0) |= (1 << 9);
	MCHBAR32(0x15f0) |= (1 << 9);

	MCHBAR32(DCC_MCHBAR) = (MCHBAR32(DCC_MCHBAR) & ~DCC_CMD_MASK) | DCC_CMD_NOP;

	u8 reg8 = pci_read_config8(PCI_DEV(0, 0, 0), 0xf0);
	pci_write_config8(PCI_DEV(0, 0, 0), 0xf0, reg8 & ~(1 << 2));
	reg8 = pci_read_config8(PCI_DEV(0, 0, 0), 0xf0);
	pci_write_config8(PCI_DEV(0, 0, 0), 0xf0, reg8 |  (1 << 2));
	udelay(2);

				  /* 5  6  7  8  9 10 11 12 */
	static const u8 wr_lut[] = { 1, 2, 3, 4, 5, 5, 6, 6 };

	const int WL = ((timings->tWL - 5) & 7) << 6;
	const int ODT_120OHMS = (1 << 9);
	const int ODS_34OHMS = (1 << 4);
	const int WR = (wr_lut[timings->tWR - 5] & 7) << 12;
	const int DLL1 = 1 << 11;
	const int CAS = ((timings->CAS - 4) & 7) << 7;
	const int INTERLEAVED = 1 << 6;/* This is READ Burst Type == interleaved. */

	int ch, r;
	FOR_EACH_POPULATED_RANK(dimms, ch, r) {
		/* We won't do this in dual-interleaved mode,
		   so don't care about the offset. */
		const u32 rankaddr = raminit_get_rank_addr(ch, r);
		printk(BIOS_DEBUG, "Performing Jedec initialization at address 0x%08x.\n", rankaddr);
		MCHBAR32(DCC_MCHBAR) = (MCHBAR32(DCC_MCHBAR) & ~DCC_SET_EREG_MASK) | DCC_SET_EREGx(2);
		read32(rankaddr | WL);
		MCHBAR32(DCC_MCHBAR) = (MCHBAR32(DCC_MCHBAR) & ~DCC_SET_EREG_MASK) | DCC_SET_EREGx(3);
		read32(rankaddr);
		MCHBAR32(DCC_MCHBAR) = (MCHBAR32(DCC_MCHBAR) & ~DCC_SET_EREG_MASK) | DCC_SET_EREGx(1);
		read32(rankaddr | ODT_120OHMS | ODS_34OHMS);
		MCHBAR32(DCC_MCHBAR) = (MCHBAR32(DCC_MCHBAR) & ~DCC_CMD_MASK) | DCC_SET_MREG;
		read32(rankaddr | WR | DLL1 | CAS | INTERLEAVED);
		MCHBAR32(DCC_MCHBAR) = (MCHBAR32(DCC_MCHBAR) & ~DCC_CMD_MASK) | DCC_SET_MREG;
		read32(rankaddr | WR        | CAS | INTERLEAVED);
	}
}

static void ddr3_calibrate_zq(void) {
	udelay(2);

	u32 tmp = MCHBAR32(DCC_MCHBAR);
	tmp &= ~(7 << 16);
	tmp |=  (5 << 16); /* ZQ calibration mode */
	MCHBAR32(DCC_MCHBAR) = tmp;

	MCHBAR32(CxDRT6_MCHBAR(0)) |= (1 << 3);
	MCHBAR32(CxDRT6_MCHBAR(1)) |= (1 << 3);

	udelay(1);

	MCHBAR32(CxDRT6_MCHBAR(0)) &= ~(1 << 3);
	MCHBAR32(CxDRT6_MCHBAR(1)) &= ~(1 << 3);

	MCHBAR32(DCC_MCHBAR) |= (7 << 16); /* Normal operation */
}

static void post_jedec_sequence(const int cores) {
	const int quadcore = cores == 4;

	MCHBAR32(0x0040) &= ~(1 << 1);
	MCHBAR32(0x0230) &= ~(3 << 1);
	MCHBAR32(0x0230) |= 1 << 15;
	MCHBAR32(0x0230) &= ~(1 << 19);
	MCHBAR32(0x1250) = 0x6c4;
	MCHBAR32(0x1350) = 0x6c4;
	MCHBAR32(0x1254) = 0x871a066d;
	MCHBAR32(0x1354) = 0x871a066d;
	MCHBAR32(0x0238) |= 1 << 26;
	MCHBAR32(0x0238) &= ~(3 << 24);
	MCHBAR32(0x0238) |= 1 << 23;
	MCHBAR32(0x0238) = (MCHBAR32(0x238) & ~(7 << 20)) | (3 << 20);
	MCHBAR32(0x0238) = (MCHBAR32(0x238) & ~(7 << 17)) | (6 << 17);
	MCHBAR32(0x0238) = (MCHBAR32(0x238) & ~(7 << 14)) | (6 << 14);
	MCHBAR32(0x0238) = (MCHBAR32(0x238) & ~(7 << 11)) | (6 << 11);
	MCHBAR32(0x0238) = (MCHBAR32(0x238) & ~(7 <<  8)) | (6 <<  8);
	MCHBAR32(0x023c) &= ~(3 << 24);
	MCHBAR32(0x023c) &= ~(1 << 23);
	MCHBAR32(0x023c) = (MCHBAR32(0x23c) & ~(7 << 20)) | (3 << 20);
	MCHBAR32(0x023c) = (MCHBAR32(0x23c) & ~(7 << 17)) | (6 << 17);
	MCHBAR32(0x023c) = (MCHBAR32(0x23c) & ~(7 << 14)) | (6 << 14);
	MCHBAR32(0x023c) = (MCHBAR32(0x23c) & ~(7 << 11)) | (6 << 11);
	MCHBAR32(0x023c) = (MCHBAR32(0x23c) & ~(7 <<  8)) | (6 <<  8);

	if (quadcore) {
		MCHBAR32(0xb14) |= (0xbfbf << 16);
	}
}

static void dram_optimizations(const timings_t *const timings,
			       const dimminfo_t *const dimms)
{
	int ch;

	FOR_EACH_POPULATED_CHANNEL(dimms, ch) {
		const unsigned int mchbar = CxDRC1_MCHBAR(ch);
		u32 cxdrc1 = MCHBAR32(mchbar);
		cxdrc1 &= ~CxDRC1_SSDS_MASK;
		if (dimms[ch].ranks == 1)
			cxdrc1 |= CxDRC1_SS;
		else
			cxdrc1 |= CxDRC1_DS;
		MCHBAR32(mchbar) = cxdrc1;
	}
}
#endif

u32 raminit_get_rank_addr(unsigned int channel, unsigned int rank)
{
	if (!channel && !rank)
		return 0; /* Address of first rank */

	/* Read the bound of the previous rank. */
	if (rank > 0) {
		rank--;
	} else {
		rank = 3; /* Highest rank per channel */
		channel--;
	}
	const u32 reg = MCHBAR32(CxDRBy_MCHBAR(channel, rank));
	/* Bound is in 32MB. */
	return ((reg & CxDRBy_BOUND_MASK(rank)) >> CxDRBy_BOUND_SHIFT(rank)) << 25;
}

void raminit_reset_readwrite_pointers(void) {
	MCHBAR32(0x1234) |=  (1 <<  6);
	MCHBAR32(0x1234) &= ~(1 <<  6);
	MCHBAR32(0x1334) |=  (1 <<  6);
	MCHBAR32(0x1334) &= ~(1 <<  6);
	MCHBAR32(0x14f0) &= ~(1 <<  9);
	MCHBAR32(0x14f0) |=  (1 <<  9);
	MCHBAR32(0x14f0) |=  (1 << 10);
	MCHBAR32(0x15f0) &= ~(1 <<  9);
	MCHBAR32(0x15f0) |=  (1 <<  9);
	MCHBAR32(0x15f0) |=  (1 << 10);
}

#if CONFIG_DEBUG_RAM_SETUP
void sdram_dump_mchbar_registers(void);

void sdram_dump_mchbar_registers(void)
{
	int i;
	printk(BIOS_DEBUG, "Dumping MCHBAR Registers\n");

	for (i=0; i<0xfff; i+=4) {
		if (MCHBAR32(i) == 0)
			continue;
		printk(BIOS_DEBUG, "0x%04x: 0x%08x\n", i, MCHBAR32(i));
	}
}
#endif

void sdram_initialize(int boot_path, sysinfo_t *sysinfo965);

static void
clkcfg1 (void)
{
// reserved
// 3878.3879    .H..    [0000:fff009a2]   MCHBAR: [00000c00] => 00020332
// 3878.387a    .H..    [0000:fff009a2]   MCHBAR: [00000c00] <= 00000332
// 3878.387b    .H..    [0000:fff009a2]   MCHBAR: [00000c00] => 00000332
	MCHBAR32(CLKCFG_MCHBAR) &= ~(2 << 16);
}

void raminit(sysinfo_t *const sysinfo, const int s3resume)
{
	const dimminfo_t *const dimms = sysinfo->dimms;
	const timings_t *const timings = &sysinfo->selected_timings;

#if 0
	const int sff = sysinfo->gfx_type == GMCH_GS45;

	int ch;
	u8 reg8;
#endif

#if 0	
// This is TPM?
	/* Wait for some bit, maybe TXT clear. */
	if (sysinfo->txt_enabled) {
		while (!(read8(0xfed40000) & (1 << 7))) {}
	}
#endif

	/* Enable SMBUS. */
	enable_smbus();

#if 0
	/* Collect information about DIMMs and find common settings. */
	collect_dimm_config(sysinfo);
#else
	sdram_initialize(0, sysinfo);
#endif

	sdram_dump_mchbar_registers();

{
	unsigned int i;

        for (i = 0; i < 2; i++) {
		printk(BIOS_SPEW, "DIMM Bank %d populated:\n"
			" byte width:    %4u\n"
			" page size:     %4u\n"
			" banks:         %4u\n"
			" ranks:         %4u\n"
			" MB:            %4u\n",
			i,
			(1 << (sysinfo->dimms[i].chip_width + 2)),
			sysinfo->dimms[i].page_size,
			sysinfo->dimms[i].banks,
			sysinfo->dimms[i].ranks,
			sysinfo->dimms[i].rank_capacity_mb);
        }

	printk(BIOS_SPEW, "Negotiated timing:\n"
		" CAS:    0x%04x\n"
		" tRAS:   0x%04x\n"
		" tRP:    0x%04x\n"
		" tRCD:   0x%04x\n"
		" tRFC:   0x%04x\n"
		" tWR:    0x%04x\n",
		sysinfo->selected_timings.CAS,
		sysinfo->selected_timings.tRAS,
		sysinfo->selected_timings.tRP,
		sysinfo->selected_timings.tRCD,
		sysinfo->selected_timings.tRFC,
		sysinfo->selected_timings.tWR);
}


	/* Check for bad warm boot. */
	reset_on_bad_warmboot();

//////////////////////////////////////
	clkcfg1();

	/* Program clock crossing registers. */
	clock_crossing_setup(timings->fsb_clock, timings->mem_clock, dimms);

	pre_jedec_settings1();
	prejedec_memory_map(dimms, timings->channel_mode);
//////////////////////////////////////

	/***** From now on, program according to collected infos: *****/

#if 0
	/* Program DRAM type. */
	switch (sysinfo->spd_type) {
	case DDR2:
		MCHBAR8(0x1434) |= (1 << 7);
		break;
	case DDR3:
		MCHBAR8(0x1434) |= (3 << 0);
		break;
	}
#endif

#if 0
	/* Program system memory frequency. */
	set_system_memory_frequency(timings);
	/* Program IGD memory frequency. */
	set_igd_memory_frequencies(sysinfo);

	/* Configure DRAM control mode for populated channels. */
	configure_dram_control_mode(timings, dimms);

	/* Initialize RCOMP. */
	rcomp_initialization(sysinfo->stepping, sff);

	/* Power-up DRAM. */
	dram_powerup(s3resume);
	/* Program DRAM timings. */
	dram_program_timings(timings);
	/* Program number of banks. */
	dram_program_banks(dimms);
	/* Enable DRAM clock pairs for populated DIMMs. */
	FOR_EACH_POPULATED_CHANNEL(dimms, ch)
		MCHBAR32(CxDCLKDIS_MCHBAR(ch)) |= CxDCLKDIS_ENABLE;

	/* Enable On-Die Termination. */
	odt_setup(timings, sff);
	/* Miscellaneous settings. */
	misc_settings(timings, sysinfo->stepping);
	/* Program clock crossing registers. */
	clock_crossing_setup(timings->fsb_clock, timings->mem_clock, dimms);
	/* Program egress VC1 timings. */
	vc1_program_timings(timings->fsb_clock);
	/* Perform system-memory i/o initialization. */
	memory_io_init(timings->mem_clock, dimms, sysinfo->stepping, sff);

	/* Initialize memory map with dummy values of 128MB per rank with a
	   page size of 4KB. This makes the JEDEC initialization code easier. */
	prejedec_memory_map(dimms, timings->channel_mode);
	if (!s3resume)
		/* Perform JEDEC initialization of DIMMS. */
		jedec_init(timings, dimms);
	/* Some programming steps after JEDEC initialization. */
	post_jedec_sequence(sysinfo->cores);

	/* Announce normal operation, initialization completed. */
	MCHBAR32(DCC_MCHBAR) |= (0x7 << 16) | (0x1 << 19);
	reg8 = pci_read_config8(PCI_DEV(0, 0, 0), 0xf0);
	pci_write_config8(PCI_DEV(0, 0, 0), 0xf0, reg8 | (1 << 2));
	reg8 = pci_read_config8(PCI_DEV(0, 0, 0), 0xf0);
	pci_write_config8(PCI_DEV(0, 0, 0), 0xf0, reg8 & ~(1 << 2));


	/* Take a breath (the reader). */


	/* Perform ZQ calibration for DDR3. */
	ddr3_calibrate_zq();

	/* Perform receive-enable calibration. */
	raminit_receive_enable_calibration(timings, dimms);
	/* Lend clock values from receive-enable calibration. */
	MCHBAR32(0x1224) = (MCHBAR32(0x1224) & ~(0xf0)) |
			   ((((MCHBAR32(0x121c) >> 7) - 1) & 0xf) << 4);
	MCHBAR32(0x1324) = (MCHBAR32(0x1324) & ~(0xf0)) |
			   ((((MCHBAR32(0x131c) >> 7) - 1) & 0xf) << 4);

	/* Perform read/write training for high clock rate. */
	if (timings->mem_clock == MEM_CLOCK_1067MT) {
		raminit_read_training(dimms, s3resume);
		raminit_write_training(timings->mem_clock, dimms, s3resume);
	}

	igd_compute_ggc(sysinfo);

	/* Program final memory map (with real values). */
	program_memory_map(dimms, timings->channel_mode, 0, sysinfo->ggc);

	/* Some last optimizations. */
	dram_optimizations(timings, dimms);

	/* Mark raminit beeing finished. :-) */
	u8 tmp8 = pci_read_config8(PCI_DEV(0, 0x1f, 0), 0xa2) & ~(1 << 7);
	pci_write_config8(PCI_DEV(0, 0x1f, 0), 0xa2, tmp8);

	raminit_thermal(sysinfo);
	init_igd(sysinfo);
#endif
}
