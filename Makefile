INSTALL = install
CFLAGS = -Wall -g
LFLAGS =
OBJS = packchr.o

prefix = /usr/local
datarootdir = $(prefix)/share
datadir = $(datarootdir)
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
infodir = $(datarootdir)/info
mandir = $(datarootdir)/man

packchr: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o packchr

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: packchr
	$(INSTALL) -m 0755 packchr $(bindir)
	$(INSTALL) -m 0444 packchr.1 $(mandir)/man1

clean:
	rm -f $(OBJS) packchr packchr.exe

.PHONY: clean install
