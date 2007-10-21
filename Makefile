packchr: packchr.o
	$(CC) packchr.o -o packchr

%.o: %.c
	$(CC) -Wall -g -c $< -o $@

.PHONY: clean

clean:
	rm -f *.o packchr packchr.exe
