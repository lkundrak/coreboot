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

/* For a detected DIMM, test the value of an SPD byte to
   match the expected value after masking some bits. */
static int test_dimm(sysinfo_t *const sysinfo,
		     int dimm, int addr, int bitmask, int expected)
{
	return (smbus_read_byte(sysinfo->spd_map[dimm], addr) & bitmask) == expected;
}

/* This function dies if dimm is unsuitable for the chipset. */
static void verify_ddr3_dimm(sysinfo_t *const sysinfo, int dimm)
{
	if (!test_dimm(sysinfo, dimm, 3, 15, 3))
		die("Chipset only supports SO-DIMM\n");

	if (!test_dimm(sysinfo, dimm, 8, 0x18, 0))
		die("Chipset doesn't support ECC RAM\n");

	if (!test_dimm(sysinfo, dimm, 7, 0x38, 0) &&
			!test_dimm(sysinfo, dimm, 7, 0x38, 8))
		die("Chipset wants single or double sided DIMMs\n");

	if (!test_dimm(sysinfo, dimm, 7, 7, 1) &&
			!test_dimm(sysinfo, dimm, 7, 7, 2))
		die("Chipset requires x8 or x16 width\n");

	if (!test_dimm(sysinfo, dimm, 4, 0x0f, 0) &&
			!test_dimm(sysinfo, dimm, 4, 0x0f, 1) &&
			!test_dimm(sysinfo, dimm, 4, 0x0f, 2) &&
			!test_dimm(sysinfo, dimm, 4, 0x0f, 3))
		die("Chipset requires 256Mb, 512Mb, 1Gb or 2Gb chips.");

	if (!test_dimm(sysinfo, dimm, 4, 0x70, 0))
		die("Chipset requires 8 banks on DDR3\n");

	/* How to check if burst length is 8?
	   Other values are not supported, are they even possible? */

	if (!test_dimm(sysinfo, dimm, 10, 0xff, 1))
		die("Code assumes 1/8ns MTB\n");

	if (!test_dimm(sysinfo, dimm, 11, 0xff, 8))
		die("Code assumes 1/8ns MTB\n");

	if (!test_dimm(sysinfo, dimm, 62, 0x9f, 0) &&
			!test_dimm(sysinfo, dimm, 62, 0x9f, 1) &&
			!test_dimm(sysinfo, dimm, 62, 0x9f, 2) &&
			!test_dimm(sysinfo, dimm, 62, 0x9f, 3) &&
			!test_dimm(sysinfo, dimm, 62, 0x9f, 5))
		die("Only raw card types A, B, C, D and F are supported.\n");
}

/* For every detected DIMM, test if it's suitable for the chipset. */
static void verify_ddr3(sysinfo_t *const sysinfo, int mask)
{
	int cur = 0;
	while (mask) {
		if (mask & 1) {
			verify_ddr3_dimm(sysinfo, cur);
		}
		mask >>= 1;
		cur++;
	}
}


typedef struct {
	int dimm_mask;
	struct {
		unsigned int rows;
		unsigned int cols;
		unsigned int chip_capacity;
		unsigned int banks;
		unsigned int ranks;
		unsigned int cas_latencies;
		unsigned int tAAmin;
		unsigned int tCKmin;
		unsigned int width;
		unsigned int tRAS;
		unsigned int tRP;
		unsigned int tRCD;
		unsigned int tWR;
		unsigned int page_size;
		unsigned int raw_card;
	} channel[2];
} spdinfo_t;
/*
 * This function collects RAM characteristics from SPD, assuming that RAM
 * is generally within chipset's requirements, since verify_ddr3() passed.
 */
static void collect_ddr3(sysinfo_t *const sysinfo, spdinfo_t *const config)
{
	int mask = config->dimm_mask;
	int cur = 0;
	while (mask != 0) {
		/* FIXME: support several dimms on same channel.  */
		if ((mask & 1) && sysinfo->spd_map[2 * cur]) {
			int tmp;
			const int smb_addr = sysinfo->spd_map[2 * cur];

			config->channel[cur].rows = ((smbus_read_byte(smb_addr, 5) >> 3) & 7) + 12;
			config->channel[cur].cols = (smbus_read_byte(smb_addr, 5) & 7) + 9;

			config->channel[cur].chip_capacity = smbus_read_byte(smb_addr, 4) & 0xf;

			config->channel[cur].banks = 8; /* GM45 only accepts this for DDR3.
							   verify_ddr3() fails for other values. */
			config->channel[cur].ranks = ((smbus_read_byte(smb_addr, 7) >> 3) & 7) + 1;

			config->channel[cur].cas_latencies =
				((smbus_read_byte(smb_addr, 15) << 8) | smbus_read_byte(smb_addr, 14))
				<< 4; /* so bit x is CAS x */
			config->channel[cur].tAAmin = smbus_read_byte(smb_addr, 16); /* in MTB */
			config->channel[cur].tCKmin = smbus_read_byte(smb_addr, 12); /* in MTB */

			config->channel[cur].width = smbus_read_byte(smb_addr, 7) & 7;
			config->channel[cur].page_size = config->channel[cur].width *
								(1 << config->channel[cur].cols); /* in Bytes */

			tmp = smbus_read_byte(smb_addr, 21);
			config->channel[cur].tRAS = smbus_read_byte(smb_addr, 22) | ((tmp & 0xf) << 8);
			config->channel[cur].tRP = smbus_read_byte(smb_addr, 20);
			config->channel[cur].tRCD = smbus_read_byte(smb_addr, 18);
			config->channel[cur].tWR = smbus_read_byte(smb_addr, 17);

			config->channel[cur].raw_card = smbus_read_byte(smb_addr, 62) & 0x1f;
		}
		cur++;
		mask >>= 2;
	}
}

#define ROUNDUP_DIV(val, by) CEIL_DIV(val, by)
#define ROUNDUP_DIV_THIS(val, by) val = ROUNDUP_DIV(val, by)
static fsb_clock_t read_fsb_clock(void)
{
	switch (MCHBAR32(CLKCFG_MCHBAR) & CLKCFG_FSBCLK_MASK) {
	case 6:
		return FSB_CLOCK_1067MHz;
	case 2:
		return FSB_CLOCK_800MHz;
	case 3:
		return FSB_CLOCK_667MHz;
	default:
		die("Unsupported FSB clock.\n");
	}
}
static mem_clock_t clock_index(const unsigned int clock)
{
	switch (clock) {
		case 533:	return MEM_CLOCK_533MHz;
		case 400:	return MEM_CLOCK_400MHz;
		case 333:	return MEM_CLOCK_333MHz;
		default:	die("Unknown clock value.\n");
	}
	return -1; /* Won't be reached. */
}
static void normalize_clock(unsigned int *const clock)
{
	if (*clock >= 533)
		*clock = 533;
	else if (*clock >= 400)
		*clock = 400;
	else if (*clock >= 333)
		*clock = 333;
	else
		*clock = 0;
}
static void lower_clock(unsigned int *const clock)
{
	--*clock;
	normalize_clock(clock);
}
static unsigned int find_common_clock_cas(sysinfo_t *const sysinfo,
					  const spdinfo_t *const spdinfo)
{
	/* various constraints must be fulfilled:
	   CAS * tCK < 20ns == 160MTB
	   tCK_max >= tCK >= tCK_min
	   CAS >= roundup(tAA_min/tCK)
	   CAS supported
	   Clock(MHz) = 1000 / tCK(ns)
	   Clock(MHz) = 8000 / tCK(MTB)
	   AND BTW: Clock(MT) = 2000 / tCK(ns) - intel uses MTs but calls them MHz
	 */
	int i;

	/* Calculate common cas_latencies mask, tCKmin and tAAmin. */
	unsigned int cas_latencies = (unsigned int)-1;
	unsigned int tCKmin = 0, tAAmin = 0;
	FOR_EACH_POPULATED_CHANNEL(sysinfo->dimms, i) {
		cas_latencies &= spdinfo->channel[i].cas_latencies;
		if (spdinfo->channel[i].tCKmin > tCKmin)
			tCKmin = spdinfo->channel[i].tCKmin;
		if (spdinfo->channel[i].tAAmin > tAAmin)
			tAAmin = spdinfo->channel[i].tAAmin;
	}

	/* Get actual value of fsb clock. */
	sysinfo->selected_timings.fsb_clock = read_fsb_clock();
	unsigned int fsb_mhz = 0;
	switch (sysinfo->selected_timings.fsb_clock) {
		case FSB_CLOCK_1067MHz:	fsb_mhz = 1067; break;
		case FSB_CLOCK_800MHz:	fsb_mhz =  800; break;
		case FSB_CLOCK_667MHz:	fsb_mhz =  667; break;
	}

	unsigned int clock = 8000 / tCKmin;
	if ((clock > sysinfo->max_ddr3_mt / 2) || (clock > fsb_mhz / 2)) {
		int new_clock = min(sysinfo->max_ddr3_mt / 2, fsb_mhz / 2);
		printk(BIOS_SPEW, "DIMMs support %d MHz, but chipset only runs at up to %d. Limiting...\n",
			clock, new_clock);
		clock = new_clock;
	}
	normalize_clock(&clock);

	/* Find compatible clock / CAS pair. */
	unsigned int tCKproposed;
	unsigned int CAS;
	while (1) {
		if (!clock)
			die("Couldn't find compatible clock / CAS settings.\n");
		tCKproposed = 8000 / clock;
		CAS = ROUNDUP_DIV(tAAmin, tCKproposed);
		printk(BIOS_SPEW, "Trying CAS %u, tCK %u.\n", CAS, tCKproposed);
		for (; CAS <= DDR3_MAX_CAS; ++CAS)
			if (cas_latencies & (1 << CAS))
				break;
		if ((CAS <= DDR3_MAX_CAS) && (CAS * tCKproposed < 160)) {
			/* Found good CAS. */
			printk(BIOS_SPEW, "Found compatible clock / CAS pair: %u / %u.\n", clock, CAS);
			break;
		}
		lower_clock(&clock);
	}
	sysinfo->selected_timings.CAS = CAS;
	sysinfo->selected_timings.mem_clock = clock_index(clock);

	return tCKproposed;
}

static void calculate_derived_timings(sysinfo_t *const sysinfo,
				      const unsigned int tCLK,
				      const spdinfo_t *const spdinfo)
{
	int i;

	/* Calculate common tRASmin, tRPmin, tRCDmin and tWRmin. */
	unsigned int tRASmin = 0, tRPmin = 0, tRCDmin = 0, tWRmin = 0;
	FOR_EACH_POPULATED_CHANNEL(sysinfo->dimms, i) {
		if (spdinfo->channel[i].tRAS > tRASmin)
			tRASmin = spdinfo->channel[i].tRAS;
		if (spdinfo->channel[i].tRP > tRPmin)
			tRPmin = spdinfo->channel[i].tRP;
		if (spdinfo->channel[i].tRCD > tRCDmin)
			tRCDmin = spdinfo->channel[i].tRCD;
		if (spdinfo->channel[i].tWR > tWRmin)
			tWRmin = spdinfo->channel[i].tWR;
	}
	ROUNDUP_DIV_THIS(tRASmin, tCLK);
	ROUNDUP_DIV_THIS(tRPmin, tCLK);
	ROUNDUP_DIV_THIS(tRCDmin, tCLK);
	ROUNDUP_DIV_THIS(tWRmin, tCLK);

	/* Lookup tRFC and calculate common tRFCmin. */
	const unsigned int tRFC_from_clock_and_cap[][4] = {
	/*             CAP_256M	   CAP_512M      CAP_1G      CAP_2G */
	/* 533MHz */ {       40,         56,         68,        104 },
	/* 400MHz */ {       30,         42,         51,         78 },
	/* 333MHz */ {       25,         35,         43,         65 },
	};
	unsigned int tRFCmin = 0;
	FOR_EACH_POPULATED_CHANNEL(sysinfo->dimms, i) {
		const unsigned int tRFC = tRFC_from_clock_and_cap
			[sysinfo->selected_timings.mem_clock][spdinfo->channel[i].chip_capacity];
		if (tRFC > tRFCmin)
			tRFCmin = tRFC;
	}

	/* Calculate common tRD from CAS and FSB and DRAM clocks. */
	unsigned int tRDmin = sysinfo->selected_timings.CAS;
	switch (sysinfo->selected_timings.fsb_clock) {
	case FSB_CLOCK_667MHz:
		tRDmin += 1;
		break;
	case FSB_CLOCK_800MHz:
		tRDmin += 2;
		break;
	case FSB_CLOCK_1067MHz:
		tRDmin += 3;
		if (sysinfo->selected_timings.mem_clock == MEM_CLOCK_1067MT)
			tRDmin += 1;
		break;
	}

	/* Calculate common tRRDmin. */
	unsigned int tRRDmin = 0;
	FOR_EACH_POPULATED_CHANNEL(sysinfo->dimms, i) {
		unsigned int tRRD = 2 + (spdinfo->channel[i].page_size / 1024);
		if (sysinfo->selected_timings.mem_clock == MEM_CLOCK_1067MT)
			tRRD += (spdinfo->channel[i].page_size / 1024);
		if (tRRD > tRRDmin)
			tRRDmin = tRRD;
	}

	/* Lookup and calculate common tFAWmin. */
	unsigned int tFAW_from_pagesize_and_clock[][3] = {
	/*		533MHz	400MHz	333MHz */
	/* 1K */ {	20,	15,	13	},
	/* 2K */ {	27,	20,	17	},
	};
	unsigned int tFAWmin = 0;
	FOR_EACH_POPULATED_CHANNEL(sysinfo->dimms, i) {
		const unsigned int tFAW = tFAW_from_pagesize_and_clock
			[spdinfo->channel[i].page_size / 1024 - 1]
			[sysinfo->selected_timings.mem_clock];
		if (tFAW > tFAWmin)
			tFAWmin = tFAW;
	}

	/* Refresh rate is fixed. */
	unsigned int tWL;
	if (sysinfo->selected_timings.mem_clock == MEM_CLOCK_1067MT) {
		tWL = 6;
	} else {
		tWL = 5;
	}

	printk(BIOS_SPEW, "Timing values:\n"
		" tCLK:  %3u\n"
		" tRAS:  %3u\n"
		" tRP:   %3u\n"
		" tRCD:  %3u\n"
		" tRFC:  %3u\n"
		" tWR:   %3u\n"
		" tRD:   %3u\n"
		" tRRD:  %3u\n"
		" tFAW:  %3u\n"
		" tWL:   %3u\n",
		tCLK, tRASmin, tRPmin, tRCDmin, tRFCmin, tWRmin, tRDmin, tRRDmin, tFAWmin, tWL);

	sysinfo->selected_timings.tRAS	= tRASmin;
	sysinfo->selected_timings.tRP	= tRPmin;
	sysinfo->selected_timings.tRCD	= tRCDmin;
	sysinfo->selected_timings.tRFC	= tRFCmin;
	sysinfo->selected_timings.tWR	= tWRmin;
	sysinfo->selected_timings.tRD	= tRDmin;
	sysinfo->selected_timings.tRRD	= tRRDmin;
	sysinfo->selected_timings.tFAW	= tFAWmin;
	sysinfo->selected_timings.tWL	= tWL;
}

static void collect_dimm_config(sysinfo_t *const sysinfo)
{
	int i;
	spdinfo_t spdinfo;

	spdinfo.dimm_mask = 0;
	sysinfo->spd_type = 0;

	for (i = 0; i < 4; i++)
		if (sysinfo->spd_map[i]) {
			const u8 spd = smbus_read_byte(sysinfo->spd_map[i], 2);
			printk (BIOS_DEBUG, "%x:%x:%x\n",
				i, sysinfo->spd_map[i],
				spd);
			if ((spd == 7) || (spd == 8) || (spd == 0xb)) {
				spdinfo.dimm_mask |= 1 << i;
				if (sysinfo->spd_type && sysinfo->spd_type != spd) {
					die("Multiple types of DIMM installed in the system, don't do that!\n");
				}
				sysinfo->spd_type = spd;
			}
		}
	if (spdinfo.dimm_mask == 0) {
		die("Could not find any DIMM.\n");
	}

	/* Normalize spd_type to 1, 2, 3. */
	sysinfo->spd_type = (sysinfo->spd_type & 1) | ((sysinfo->spd_type & 8) >> 2);
	printk(BIOS_SPEW, "DDR mask %x, DDR %d\n", spdinfo.dimm_mask, sysinfo->spd_type);

	if (sysinfo->spd_type == DDR2) {
		die("DDR2 not supported at this time.\n");
	} else if (sysinfo->spd_type == DDR3) {
		verify_ddr3(sysinfo, spdinfo.dimm_mask);
		collect_ddr3(sysinfo, &spdinfo);
	} else {
		die("Will never support DDR1.\n");
	}

	for (i = 0; i < 2; i++) {
		if ((spdinfo.dimm_mask >> (i*2)) & 1) {
			printk(BIOS_SPEW, "Bank %d populated:\n"
					  " Raw card type: %4c\n"
					  " Row addr bits: %4u\n"
					  " Col addr bits: %4u\n"
					  " byte width:    %4u\n"
					  " page size:     %4u\n"
					  " banks:         %4u\n"
					  " ranks:         %4u\n"
					  " tAAmin:    %3u\n"
					  " tCKmin:    %3u\n"
					  "  Max clock: %3u MHz\n"
					  " CAS:       0x%04x\n",
				i, spdinfo.channel[i].raw_card + 'A',
				spdinfo.channel[i].rows, spdinfo.channel[i].cols,
				spdinfo.channel[i].width, spdinfo.channel[i].page_size,
				spdinfo.channel[i].banks, spdinfo.channel[i].ranks,
				spdinfo.channel[i].tAAmin, spdinfo.channel[i].tCKmin,
				8000 / spdinfo.channel[i].tCKmin, spdinfo.channel[i].cas_latencies);
		}
	}

	FOR_EACH_CHANNEL(i) {
		sysinfo->dimms[i].card_type =
			(spdinfo.dimm_mask & (1 << (i * 2))) ? spdinfo.channel[i].raw_card + 0xa : 0;
	}

	/* Find common memory clock and CAS. */
	const unsigned int tCLK = find_common_clock_cas(sysinfo, &spdinfo);

	/* Calculate other timings from clock and CAS. */
	calculate_derived_timings(sysinfo, tCLK, &spdinfo);

	/* Initialize DIMM infos. */
	/* Always prefer interleaved over async channel mode. */
	FOR_EACH_CHANNEL(i) {
		IF_CHANNEL_POPULATED(sysinfo->dimms, i) {
			sysinfo->dimms[i].banks = spdinfo.channel[i].banks;
			sysinfo->dimms[i].ranks = spdinfo.channel[i].ranks;

			/* .width is 1 for x8 or 2 for x16, bus width is 8 bytes. */
			const unsigned int chips_per_rank = 8 / spdinfo.channel[i].width;

			sysinfo->dimms[i].chip_width = spdinfo.channel[i].width;
			sysinfo->dimms[i].chip_capacity = spdinfo.channel[i].chip_capacity;
			sysinfo->dimms[i].page_size = spdinfo.channel[i].page_size * chips_per_rank;
			sysinfo->dimms[i].rank_capacity_mb =
				/* offset of chip_capacity is 8 (256M), therefore, add 8
				   chip_capacity is in Mbit, we want MByte, therefore, subtract 3 */
				(1 << (spdinfo.channel[i].chip_capacity + 8 - 3)) * chips_per_rank;
		}
	}
	if (CHANNEL_IS_POPULATED(sysinfo->dimms, 0) &&
			CHANNEL_IS_POPULATED(sysinfo->dimms, 1))
		sysinfo->selected_timings.channel_mode = CHANNEL_MODE_DUAL_INTERLEAVED;
	else
		sysinfo->selected_timings.channel_mode = CHANNEL_MODE_SINGLE;
}
