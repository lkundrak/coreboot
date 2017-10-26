#include <arch/io.h>

static void bootblock_mainboard_init(void)
{
        //if (CONFIG_ROM_SIZE == 0x200000)
	pci_io_write_config8(PCI_DEV(0, 0x11, 0), 0x40, 0x04);
	pci_io_write_config8(PCI_DEV(0, 0x11, 0), 0x41, 0xc0);
}
