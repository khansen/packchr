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

.PHONY: clean

clean:
	rm -f *.o packchr packchr.exe
