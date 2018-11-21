#include "obc.h"


int main(void) {
    init_obc_core();

    while (1) {
        execute_next_cmd();
    }

    return 0;
}
