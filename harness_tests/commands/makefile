# This makefile should go in a specific test folder within examples,
# harness_tests, or manual_tests, e.g. `manual_tests/commands_test/makefile`

PROG = main1
# .c files from `src` folder to compile (except for in `lib-common`),
# separated by spaces
SRC = $(addprefix ../../src/, commands.c command_utilities.c general.c transceiver.c mem.c can_interface.c rtc.c can_commands.c i2c.c)
include ../makefile
