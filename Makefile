CC	= gbdk-2020/build/gbdk/bin/lcc -Wa-l -Wl-m

.PHONY: all
all:	main.gb
	rm -f *.lnk

%.s:	%.c
	$(CC) -S -o $@ $<

%.o:	%.c
	$(CC) $(RELEASE) -c -o $@ $<

%.o:	%.s
	$(CC) -c -o $@ $<

%.gb:	%.o
	$(CC) -o $@ $<

clean:
	rm -f *.o *.lst *.map *.gb *.ihx *.sym *.cdb *.adb *.asm
