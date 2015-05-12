set -e
if make
then
	time notify-send Finished "$(
		set +x
		scp build/coreboot.rom malina.local: 2>&1 |tee /dev/stderr
		ssh root@malina.local '~lkundrak/src/flashrom-0.9.7/flashrom  -p linux_spi:dev=/dev/spidev0.0 -w ~lkundrak/coreboot.rom  -c MX25L3206E -n' 2>&1 |tee /dev/stderr
	)"
else
	notify-send Hapalo
fi
