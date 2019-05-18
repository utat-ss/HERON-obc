# This makefile should go in a specific test folder within examples,
# harness_tests, or manual_tests, e.g. `manual_tests/commands_test/makefile`

PROG = main1
# .c files from `src` folder to compile (except for in `lib-common`),
# separated by spaces
SRC = $(addprefix ../../src/, can_commands.c can_interface.c commands.c general.c mem.c rtc.c trans_commands.c transceiver.c)
include ../makefile