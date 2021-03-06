/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 Advanced Micro Devices, Inc.
 * Copyright (C) 2014 Sage Electronic Engineering, LLC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <arch/io.h>
#include <cpu/x86/msr.h>
#include <device/pci_def.h>
#include <southbridge/amd/amd_pci_util.h>
#include <southbridge/amd/cimx/cimx_util.h>
#include <arch/acpi.h>
#include "BiosCallOuts.h"
#include <cpu/amd/agesa/s3_resume.h>
#include <cpu/amd/mtrr.h>
#include "SBPLATFORM.h"
#include <southbridge/amd/cimx/sb800/pci_devs.h>
#include <northbridge/amd/agesa/family14/pci_devs.h>

void set_pcie_reset(void);
void set_pcie_dereset(void);

/***********************************************************
 * These arrays set up the FCH PCI_INTR registers 0xC00/0xC01.
 * This table is responsible for physically routing the PIC and
 * IOAPIC IRQs to the different PCI devices on the system.  It
 * is read and written via registers 0xC00/0xC01 as an
 * Index/Data pair.  These values are chipset and mainboard
 * dependent and should be updated accordingly.
 *
 * These values are used by the PCI configuration space,
 * MP Tables.  TODO: Make ACPI use these values too.
 *
 * The Persimmon PCI INTA/B/C/D pins are connected to
 * FCH pins INTE/F/G/H on the schematic so these need
 * to be routed as well.
 */
static const u8 mainboard_picr_data[FCH_INT_TABLE_SIZE] = {
	/* INTA# - INTH# */
	[0x00] = 0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,
	/* Misc-nil,0,1,2, INT from Serial irq */
	[0x08] = 0x00,0xF0,0x00,0x00,0x1F,0x1F,0x1F,0x1F,
	/* SCI, SMBUS0, ASF, HDA, FC, GEC, PerfMon */
	[0x10] = 0x1F,0x1F,0x1F,0x0A,0x1F,0x1F,0x1F,
	/* IMC INT0 - 5 */
	[0x20] = 0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,
	/* USB Devs 18/19/20/22 INTA-C */
	[0x30] = 0x0A,0x0B,0x0A,0x0B,0x0A,0x0B,0x0A,
	/* IDE, SATA */
	[0x40] = 0x0B,0x0B,
	/* GPPInt0 - 3 */
	[0x50] = 0x0A,0x0B,0x0A,0x0B
};

static const u8 mainboard_intr_data[FCH_INT_TABLE_SIZE] = {
	/* INTA# - INTH# */
	[0x00] = 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
	/* Misc-nil,0,1,2, INT from Serial irq */
	[0x08] = 0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F,0x1F,
	/* SCI, SMBUS0, ASF, HDA, FC, GEC, PerMon */
	[0x10] = 0x09,0x1F,0x1F,0x10,0x1F,0x12,0x1F,
	/* IMC INT0 - 5 */
	[0x20] = 0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,
	/* USB Devs 18/19/22/20 INTA-C */
	[0x30] = 0x12,0x11,0x12,0x11,0x12,0x11,0x12,
	/* IDE, SATA */
	[0x40] = 0x11,0x13,
	/* GPPInt0 - 3 */
	[0x50] = 0x10,0x11,0x12,0x13
};

/*
 * This table defines the index into the picr/intr_data
 * tables for each device.  Any enabled device and slot
 * that uses hardware interrupts should have an entry
 * in this table to define its index into the FCH
 * PCI_INTR register 0xC00/0xC01.  This index will define
 * the interrupt that it should use.  Putting PIRQ_A into
 * the PIN A index for a device will tell that device to
 * use PIC IRQ 10 if it uses PIN A for its hardware INT.
 */
/*
 * Persimmon has PCI slot INTA/B/C/D connected to PIRQE/F/G/H
 * but because PCI INT_PIN swizzling isnt implemented to match
 * the IDSEL (dev 3) of the slot, the table is adjusted for the
 * swizzle and INTA is connected to PIRQH so PINA/B/C/D on
 * off-chip devices should get mapped to PIRQH/E/F/G.
 */
static const struct pirq_struct mainboard_pirq_data[] = {
	/* {PCI_devfn,        {PIN A, PIN B, PIN C, PIN D}}, */
	{GFX_DEVFN,           {PIRQ_A, PIRQ_B, PIRQ_NC, PIRQ_NC}},      /* VGA:       01.0 */
	{NB_PCIE_PORT1_DEVFN, {PIRQ_A, PIRQ_B, PIRQ_C, PIRQ_D}},        /* NIC:       04.0 */
	{NB_PCIE_PORT3_DEVFN, {PIRQ_A, PIRQ_B, PIRQ_C, PIRQ_D}},        /* PCIe bdg:  06.0 */
	{SATA_DEVFN,          {PIRQ_SATA, PIRQ_NC, PIRQ_NC, PIRQ_NC}},  /* SATA:      11.0 */
	{OHCI1_DEVFN,         {PIRQ_OHCI1, PIRQ_NC, PIRQ_NC, PIRQ_NC}}, /* OHCI1:     12.0 */
	{EHCI1_DEVFN,         {PIRQ_NC, PIRQ_EHCI1, PIRQ_NC, PIRQ_NC}}, /* EHCI1:     12.2 */
	{OHCI2_DEVFN,         {PIRQ_OHCI2, PIRQ_NC, PIRQ_NC, PIRQ_NC}}, /* OHCI2:     13.0 */
	{EHCI2_DEVFN,         {PIRQ_NC, PIRQ_EHCI2, PIRQ_NC, PIRQ_NC}}, /* EHCI2:     13.2 */
	{SMBUS_DEVFN,         {PIRQ_SMBUS, PIRQ_NC, PIRQ_NC, PIRQ_NC}}, /* SMBUS:     14.0 */
	{IDE_DEVFN,           {PIRQ_NC, PIRQ_IDE, PIRQ_NC, PIRQ_NC}},   /* IDE:       14.1 */
	{HDA_DEVFN,           {PIRQ_HDA, PIRQ_NC, PIRQ_NC, PIRQ_NC}},   /* HDA:       14.2 */
	{SB_PCI_PORT_DEVFN,   {PIRQ_H, PIRQ_E, PIRQ_F, PIRQ_G}},        /* PCI bdg:   14.4 */
	{OHCI4_DEVFN,         {PIRQ_NC, PIRQ_NC, PIRQ_OHCI4, PIRQ_NC}}, /* OHCI4:     14.5 */
	{OHCI3_DEVFN,         {PIRQ_OHCI3, PIRQ_NC, PIRQ_NC, PIRQ_NC}}, /* OHCI3:     16.0 */
	{EHCI3_DEVFN,         {PIRQ_NC, PIRQ_EHCI3, PIRQ_NC, PIRQ_NC}}, /* EHCI3:     16.2 */
};

/* PIRQ Setup */
static void pirq_setup(void)
{
	pirq_data_ptr = mainboard_pirq_data;
	pirq_data_size = sizeof(mainboard_pirq_data) / sizeof(struct pirq_struct);
	intr_data_ptr = mainboard_intr_data;
	picr_data_ptr = mainboard_picr_data;
}

/**
 * TODO
 * SB CIMx callback
 */
void set_pcie_reset(void)
{
}

/**
 * TODO
 * mainboard specific SB CIMx callback
 */
void set_pcie_dereset(void)
{
}


/**********************************************
 * Enable the dedicated functions of the board.
 **********************************************/
static void mainboard_enable(device_t dev)
{
	printk(BIOS_INFO, "Mainboard " CONFIG_MAINBOARD_PART_NUMBER " Enable.\n");

	/* enable GPP CLK0 thru CLK1 */
	/* disable GPP CLK2 thru SLT_GFX_CLK */
	u8 *misc_mem_clk_cntrl = (u8 *)(ACPI_MMIO_BASE + MISC_BASE);
	*(misc_mem_clk_cntrl + 0) = 0xFF;
	*(misc_mem_clk_cntrl + 1) = 0x00;
	*(misc_mem_clk_cntrl + 2) = 0x00;
	*(misc_mem_clk_cntrl + 3) = 0x00;
	*(misc_mem_clk_cntrl + 4) = 0x00;

	/*
	 * Initialize ASF registers to an arbitrary address because someone
	 * long ago set things up this way inside the SPD read code.  The
	 * SPD read code has been made generic and moved out of the board
	 * directory, so the ASF init is being done here.
	 */
	pm_iowrite(0x29, 0x80);
	pm_iowrite(0x28, 0x61);

	/* Initialize the PIRQ data structures for consumption */
	pirq_setup();
}

struct chip_operations mainboard_ops = {
	.enable_dev = mainboard_enable,
};
