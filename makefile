CC = avr-gcc
CFLAGS = -g -std=gnu99 -mmcu=atmega32m1 -Os -mcall-prologues
PROG = stk500# the Pololu acts like the STK500
MCU = m32m1

# Change this line depending on your OS and the port used.
PORT = COM8          

SRC = $(wildcard src/*.c)
SRC+= $(wildcard lib-common/*.c)
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)

obc: $(OBJ)
	$(CC) $(CFLAGS) -o ./build/$@.elf $^
	avr-objcopy -j .text -j .data -O ihex ./build/$@.elf ./build/$@.hex

-include $(DEP)

%.d: %.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean upload

clean:
	rm -f $(OBJ)
	rm -f $(DEP)
	rm -f ./build/*

upload: obc
	avrdude -p $(MCU) -c $(PROG) -P $(PORT) -U flash:w:./build/$^.hex
