CC = avr-gcc
INCLUDES = -I../../lib-common/include
LIB = -L../../lib-common/lib -luart -lspi -lcan -lqueue
CFLAGS = -std=gnu99 -Wall -g -mmcu=atmega32m1 -Os -mcall-prologues

PGMR = stk500
MCU = m32m1

# Computer port that the programmer is connected to
# May need to change the PORT definition based on your OS and port
# Operating system detection based on https://gist.github.com/sighingnow/deee806603ec9274fd47

# Windows
ifeq ($(OS),Windows_NT)
	# One programmer should give 2 ports (either COM3 and COM4 or COM5 and COM6)
	# Programming port is the higher number (COM4 or COM6)
	# Use powershell, list port names, get the second number in the list
	PORT = $(shell powershell "[System.IO.Ports.SerialPort]::getportnames() | select -First 2 | select -Last 1")
	# PORT = COM3
	# PORT = COM5

else
	# If on Unix, get the operating system
	UNAME_S := $(shell uname -s)

	# Linux
	ifeq ($(UNAME_S),Linux)
		PORT = /dev/ttyS3
		# PORT = /dev/ttyS5
	endif

	# macOS
	ifeq ($(UNAME_S),Darwin)
		# Automatically find the port (lower number)
		PORT = $(shell find /dev -name 'tty.usbmodem[0-9]*' | sort | head -n1)

		# Manual ports
		# PORT = /dev/tty.usbmodem00187462
		# PORT = /dev/tty.usbmodem00208212
	endif
endif

OBJ = $(SRC:../../src%.c=..%.o)

$(PROG): $(PROG).o $(OBJ)
	$(CC) $(CFLAGS) -o $@.elf $^ $(LIB)
	avr-objcopy -j .text -j .data -O ihex $@.elf $@.hex

$(PROG).o: $(PROG).c
	$(CC) $(CFLAGS) -c $(PROG).c $(INCLUDES)

../%.o: ../../src/%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES)

.PHONY: clean upload

clean:
	rm -f ../*.o
	rm -f ./*.o
	rm -f ./*.elf
	rm -f ./*.hex

upload: $(PROG)
	avrdude -p $(MCU) -c $(PGMR) -P $(PORT) -U flash:w:./$^.hex
