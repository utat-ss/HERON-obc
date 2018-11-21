#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/rtc.h"



uint8_t handle_uart_cmd(const uint8_t* data, uint8_t len) {
    // print("\n%s\n", __FUNCTION__);
    // print_hex_bytes((uint8_t *) data, len);
    for (uint8_t i = 0; i < len; i++) {
        print("%c", data[i]);
    }
    print("\n");

    if (data[len - 1] == '\n') {
        for (uint8_t i = 0; i < CMD_LIST_LEN; i++) {
            cmd_t* cmd = cmd_list[i];
            uint8_t cmd_len = strlen(cmd->cmd);

            if (len < cmd_len) continue;

            // FIXME: might be len - 1 below
            uint8_t match = 1;
            for (uint8_t i = 0; i < cmd_len; i++) {
                if (data[i] != (cmd->cmd)[i]) {
                    match = 0;
                    break;
                }
            }

            if (match) {
                // WOW: just enqueue the command directly into the queue!
                enqueue(&command_queue, (uint8_t*)&(cmd -> fn));
                // print("Enqueued %s\n", cmd->cmd);
                // Downside, can't actually pass the data/len through;
                // this means we can't support variable sized commands
                break;
                // break if we match; the input can match at most 1 command
            }
        }

        return len;
    } else {
        return 0;
    }
}


void print_can_data(void) {
    // Payload
    print("%d C\n", (int16_t) temp_convert_raw_data_to_temperature(pay_hk_data[CAN_PAY_HK_TEMP]));
    print("%d %%RH\n", (int16_t) hum_convert_raw_humidity_to_humidity(pay_hk_data[CAN_PAY_HK_HUMID]));
    print("%d kPa\n", (int16_t) pres_convert_raw_pressure_to_pressure(pay_hk_data[CAN_PAY_HK_PRES]));

    print("\n");

    // Science
    for (uint8_t i = 0; i < CAN_PAY_SCI_FIELD_COUNT; i++) {
        print("#%u: %d%%\n", i + 1, (int16_t) ((double) pay_sci_data[i] / (double) 0xFFFFFF * 100.0));
    }

    print("\n");

    // Power
    print("Bat\n");
    print("+: %d mV\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BATT_VPOS_CH]) * 1000.0));
    print("-: %d mV\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BATT_VNEG_CH]) * 1000.0));
    print("%d mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BATT_IOUT_CH]) * 1000.0));
    // Converters
    print("BB: %f mV\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BB_VOUT_CH]) * 1000.0));
    print("BT: %f mV\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BT_VOUT_CH]) * 1000.0));
    print("BB1: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BB1_IOUT_CH]) * 1000.0));
    print("BB2: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BB2_IOUT_CH]) * 1000.0));
    print("BT1: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BT1_IOUT_CH]) * 1000.0));
    print("PV\n");
    print("+X: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_PV_POS_X_IOUT_CH]) * 1000.0));
    print("-X: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_PV_NEG_X_IOUT_CH]) * 1000.0));
    print("+Y: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_PV_POS_Y_IOUT_CH]) * 1000.0));
    print("-Y: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_PV_NEG_Y_IOUT_CH]) * 1000.0));

    print("\n");
}

void print_raw_can_data(void) {
    // print("\n");
    //
    // print("PAY_HK\n");
    // for (uint8_t i = 0; i < CAN_PAY_HK_FIELD_COUNT; ++i) {
    //     print("#%u: %06x\n", i, pay_hk_data[i]);
    // }
    //
    // print("PAY_SCI\n");
    // for (uint8_t i = 0; i < CAN_PAY_SCI_FIELD_COUNT; ++i) {
    //     print("#%u: %06x\n", i, pay_sci_data[i]);
    // }
    //
    // print("EPS_HK\n");
    // for (uint8_t i = 0; i < EPS_HK_FIELD_COUNT; ++i) {
    //     print("#%u: %06x\n", i, eps_hk_data[i]);
    // }
    //
    // print("\n");
}


int main(void){
    register_callback(handle_uart_cmd);

}
