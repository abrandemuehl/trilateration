app.bin: CC = xtensa-lx106-elf-gcc
app.bin: CFLAGS = -I. -mlongcalls -nostdlib
app.bin: CXX = xtensa-lx106-elf-g++
app.bin: CXXFLAGS = $(CFLAGS) -fno-exceptions
app.bin: LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -Wl,--end-group -lgcc -lm
app.bin: LDFLAGS = -Teagle.app.v6.ld

test: CC = gcc
test: LDFLAGS = -lm

MAIN = app
TEST = test
EXES = app test
BINDIR = bin

app.bin: $(MAIN)
	esptool.py elf2image --output $(BINDIR)/$^- $^

flash: $(BINDIR)/app.bin
	# SDK 0.9.4
	esptool.py write_flash 0 $(BINDIR)/app-0x00000.bin 0x40000 $(BINDIR)/app-0x40000.bin
	# SDK 2.0.0
	# esptool.py write_flash 0 $(BINDIR)/app-0x00000.bin 0x10000 $(BINDIR)/app-0x10000.bin

$(TEST): test.c trilateration.o

trilateration.o: trilateration.c trilateration.h

$(MAIN): app.o

app.o: app.c

clean:
	rm -f $(EXES) *.o $(BINDIR)/*
