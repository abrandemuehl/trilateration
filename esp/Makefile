CC = xtensa-lx106-elf-gcc
CFLAGS = -I. -mlongcalls -nostdlib
CXX = xtensa-lx106-elf-g++
CXXFLAGS = $(CFLAGS) -fno-exceptions
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lcirom -lmirom -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld

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

../common/trilateration_esp.o: ../common/trilateration.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $^ -o $@ $(LDLIBS)

app: app.o uart.o ../common/messages.o ringbuffer.o

app.o: app.c

uart.o: uart.c

ringbuffer.o: ringbuffer.c

../common/messages.o: ../common/messages.c

$(BINDIR)/speed_test.bin: speed_test
	esptool.py elf2image --output $(BINDIR)/$^- $^

flash_speed_test: $(BINDIR)/speed_test.bin
	esptool.py write_flash 0 $(BINDIR)/speed_test-0x00000.bin 0x40000 $(BINDIR)/speed_test-0x40000.bin

speed_test: speed_test.o ../common/trilateration_esp.o

speed_test.o: speed_test.c


clean:
	rm -f $(EXES) *.o $(BINDIR)/*
