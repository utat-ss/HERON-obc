# This makefile should go in a specific test folder within examples,
# harness_tests, or manual_tests, e.g. `manual_tests/commands_test/makefile`

PROG = main1
# .c files from `src` folder to compile (except for in `lib-common`),
# separated by spaces
SRC = $(addprefix ../../src/, antenna.c can_commands.c can_interface.c command_utilities.c commands.c general.c i2c.c mem.c rtc.c transceiver.c)
include ../makefile
