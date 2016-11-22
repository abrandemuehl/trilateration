app.bin: CC = xtensa-lx106-elf-gcc
app.bin: CFLAGS = -I. -mlongcalls
app.bin: LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -Wl,--end-group -lgcc -lm
app.bin: LDFLAGS = -Teagle.app.v6.ld

test: CC = gcc
test: LDFLAGS = -lm


app.bin: app
	esptool.py elf2image $^

flash: app.bin
	esptool.py write_flash 0 app-0x00000.bin 0x10000 app-0x10000.bin

test: test.c trilateration.o

trilateration.o: trilateration.c trilateration.h

app: app.o

app.o: app.c

clean:
	rm -f main *.o *.bin
