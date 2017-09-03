#include <avr/io.h>
#include <util/delay.h>

#include "../lib-common/spi.h"
#include "../lib-common/uart.h"
#include "../lib-common/log.h"

int main(void) {
    init_uart();

    char msg[] = "This is an OBC board\n";

    for (;;) {
        print("%s", msg);
        _delay_ms(10000);
    }
}
