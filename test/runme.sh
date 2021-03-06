set -x
set -e

# decode-dimms --merge-cells --format --side-by-side -x $(ls *.hex |sort -n) >dimms.html
# html2ps dimms.html >dimms.ps ; ps2pdf dimms.ps; evince dimms.pdf

W=-Werror
sed '/CONFIG_MMCONF_BASE_ADDRESS/d' -i build/config.h
cc -DUSTEST -O0 -m32 -D__PRE_RAM__ -fno-builtin -include src/include/kconfig.h -Ibuild -Isrc/arch/x86/include -Isrc/include -DCONFIG_MMCONF_BASE_ADDRESS=0x10000000 -DDEFAULT_MCHBAR=0x20000000 -Isrc src/northbridge/intel/i965/raminit.c src/northbridge/intel/i965/ddr2.c src/northbridge/intel/i965/ram_calc.c src/lib/clog2.c test/emu.c -Wall -g3 -o test/emu $W

#TESTBASE=6
TESTBASE=67
#TESTBASE=76
#TESTBASE=65
# xxx
#TESTBASE=45

[ "$1" ] && TESTBASE="$1"
if eval test/emu $(echo "test/dimms/$(echo $TESTBASE |sed 's/\(.\)\(.\)/\1,\2/g;s/\(...*\)/{\1}/')") >test/OUT
then
	eval perl test/mchinit.pl test/initial-mchbar.c test/MCHINIT$TESTBASE test/OUT
#	perl test/mchinit.pl test/initial-mchbar.c test/MCHINIT test/OUT
else
	cat test/OUT
	false
fi

#gcc -Isrc -Isrc/include -Ibuild -Isrc/device/oprom/include -include src/include/kconfig.h -Isrc/arch/x86/include -D__PRE_RAM__ -Os -pipe -g -nostdinc -nostdlib -Wall -Wundef -Wstrict-prototypes -Wmissing-prototypes -Wwrite-strings -Wredundant-decls -Wno-trigraphs -Wstrict-aliasing -Wshadow -Werror -fno-common -fno-builtin -m32 -Wl,-b,elf32-i386 -Wl,-melf_i386 -Wno-unused-but-set-variable -Wa,--divide -march=i686 src/northbridge/intel/i965/raminit.c

# + gcc -MMD -Isrc -Isrc/include -Ibuild -Isrc/device/oprom/include -include src/include/kconfig.h -Isrc/arch/x86/include -D__PRE_RAM__ -Os -pipe -g -nostdinc -nostdlib -Wall -Wundef -Wstrict-prototypes -Wmissing-prototypes -Wwrite-strings -Wredundant-decls -Wno-trigraphs -Wstrict-aliasing -Wshadow -Werror -fno-common -ffreestanding -fno-builtin -fomit-frame-pointer -m32 -Wl,-b,elf32-i386 -Wl,-melf_i386 -Wno-unused-but-set-variable -fuse-ld=bfd -fno-stack-protector -Wl,--build-id=none -Wa,--divide -march=i686 -c -o build/northbridge/intel/i965/raminit.romstage.o src/northbridge/intel/i965/raminit.c

#src/northbridge/intel/i965/ram_calc.c ins.c
