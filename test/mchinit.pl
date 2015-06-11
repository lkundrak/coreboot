#!/usr/bin/perl

open (F, $ARGV[0]) or die "$ARGV[0]: $!";
while (<F>) {
	/MCHBAR: \[0000(.+)] <= (\S+)/ or next;
	$regs{$1} ||= [" " x 10, " " x 10];
	$regs{$1}->[0] = sprintf '%08x', hex $2;
}

open (F, $ARGV[1]) or die "$ARGV[1]: $!";
while (<F>) {
	/^0x(.{4}): (0x.{8})$/ or next;
	$regs{$1} ||= [" " x 10, " " x 10];
	$regs{$1}->[1] = sprintf '%08x', hex $2;
}

printf "0x$_: %s %s\n", @{$regs{$_}} foreach sort keys %regs;
