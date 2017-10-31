#
# Copyright (C) 2017 Lubomir Rintel <lkundrak@v3.sk>
#
# Based on an inteltool Makefile:
#
# Copyright (C) 2008 by coresystems GmbH
# written by Stefan Reinauer <stepan@coresystems.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#

PROGRAM = pirdump

all: $(PROGRAM)

clean:
	rm -f $(PROGRAM) *.o

install: $(PROGRAM)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install $(PROGRAM) $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	install -p -m644 $(PROGRAM).1 $(DESTDIR)$(PREFIX)/share/man/man1

.PHONY: all clean
