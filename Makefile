app: CC = xtensa-lx106-elf-gcc
app: CFLAGS = -I. -mlongcalls -nostdlib
app: CXX = xtensa-lx106-elf-g++
app: CXXFLAGS = $(CFLAGS) -fno-exceptions
app: LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -Wl,--end-group -lgcc -lm
app: LDFLAGS = -Teagle.app.v6.ld

test: CC = gcc
test: LDFLAGS = -lm

EXES = app test
BINDIR = bin

$(BINDIR)/app.bin: app
	esptool.py elf2image --output $(BINDIR)/$^- $^

flash: $(BINDIR)/app.bin
	# SDK 0.9.4
	esptool.py write_flash 0 $(BINDIR)/app-0x00000.bin 0x40000 $(BINDIR)/app-0x40000.bin
	# SDK 2.0.0
	# esptool.py write_flash 0 $(BINDIR)/app-0x00000.bin 0x10000 $(BINDIR)/app-0x10000.bin

test: test.c trilateration.o

trilateration.o: trilateration.c trilateration.h

app: app.o uart.o messages.o ringbuffer.o

app.o: app.c

uart.o: uart.c

ringbuffer.o: ringbuffer.c

messages.o: messages.c

clean:
	rm -f $(EXES) *.o $(BINDIR)/*
