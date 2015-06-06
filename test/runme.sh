set -x
set -e

# decode-dimms --merge-cells --format --side-by-side -x $(ls *.hex |sort -n) >dimms.html

W=-Werror
cc -O0 -m32 -D__PRE_RAM__ -fno-builtin -include src/include/kconfig.h -Ibuild -Isrc/arch/x86/include -Isrc/include -Isrc src/northbridge/intel/i965/raminit.c src/northbridge/intel/i965/ddr2.c src/northbridge/intel/i965/ram_calc.c src/lib/clog2.c test/emu.c -Wall -g3 -o test/emu $W

#exec valgrind --db-attach=no test/emu test/dimms/[67]
exec test/emu test/dimms/[67]

#gcc -Isrc -Isrc/include -Ibuild -Isrc/device/oprom/include -include src/include/kconfig.h -Isrc/arch/x86/include -D__PRE_RAM__ -Os -pipe -g -nostdinc -nostdlib -Wall -Wundef -Wstrict-prototypes -Wmissing-prototypes -Wwrite-strings -Wredundant-decls -Wno-trigraphs -Wstrict-aliasing -Wshadow -Werror -fno-common -fno-builtin -m32 -Wl,-b,elf32-i386 -Wl,-melf_i386 -Wno-unused-but-set-variable -Wa,--divide -march=i686 src/northbridge/intel/i965/raminit.c

# + gcc -MMD -Isrc -Isrc/include -Ibuild -Isrc/device/oprom/include -include src/include/kconfig.h -Isrc/arch/x86/include -D__PRE_RAM__ -Os -pipe -g -nostdinc -nostdlib -Wall -Wundef -Wstrict-prototypes -Wmissing-prototypes -Wwrite-strings -Wredundant-decls -Wno-trigraphs -Wstrict-aliasing -Wshadow -Werror -fno-common -ffreestanding -fno-builtin -fomit-frame-pointer -m32 -Wl,-b,elf32-i386 -Wl,-melf_i386 -Wno-unused-but-set-variable -fuse-ld=bfd -fno-stack-protector -Wl,--build-id=none -Wa,--divide -march=i686 -c -o build/northbridge/intel/i965/raminit.romstage.o src/northbridge/intel/i965/raminit.c

#src/northbridge/intel/i965/ram_calc.c ins.c
