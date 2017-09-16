CC = avr-gcc
CFLAGS = -g -std=gnu99 -mmcu=atmega32m1 -Os -mcall-prologues
PROG = stk500# the Pololu acts like the STK500
MCU = m32m1

INCLUDES = -I./lib-common/include/
LIB = -L./lib-common/lib/ -lspi -luart -lcan

# Change this line depending on your OS and the port used.
PORT = COM3

SRC = $(wildcard ./src/*.c)
OBJ = $(SRC:./src%.c=./build%.o)
DEP = $(OBJ:.o=.d)

obc: $(OBJ)
	$(CC) $(CFLAGS) -o ./build/$@.elf $(OBJ) $(LIB)
	avr-objcopy -j .text -j .data -O ihex ./build/$@.elf ./build/$@.hex

./build/%.o: ./src/%.c
	$(CC) $(CFLAGS) -o $@ -c $^ $(INCLUDES)

-include $(DEP)

./build/%.d: ./src/%.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean upload

clean:
	rm -f $(OBJ)
	rm -f $(DEP)

upload: obc
	avrdude -p $(MCU) -c $(PROG) -P $(PORT) -U flash:w:./build/$^.hex
