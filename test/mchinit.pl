#!/usr/bin/perl

open (F, $ARGV[0]) or die "$ARGV[0]: $!";
while (<F>) {
	/MCHBAR: \[0000(.+)] <= (\S+)/ or next;

	$off = (hex $1) % 4;
	$base = (hex $1) - $off;

	$regs{$base} ||= [0, 0];
	$regs{$base}->[0] |= (hex $2) << (8 * $off);
}

open (F, $ARGV[1]) or die "$ARGV[1]: $!";
while (<F>) {
	/^0x(.{4}): (0x.{8})$/ or next;
	$regs{$base} ||= [0, 0];
	$regs{hex $1}->[1] = hex $2;
}

$regs{$_}->[0] = $regs{$_}->[0] ? sprintf "0x%08x", $regs{$_}->[0] : " " x 10 foreach keys %regs;
$regs{$_}->[1] = $regs{$_}->[1] ? sprintf "0x%08x", $regs{$_}->[1] : " " x 10 foreach keys %regs;
printf "0x%04x: %s %s%s\n", $_, @{$regs{$_}}, (($regs{$_}->[1] =~ /\s/ or $regs{$_}->[0] eq $regs{$_}->[1]) ? '' : ' -') foreach sort keys %regs;
