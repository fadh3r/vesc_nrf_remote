#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#include "refactoring.h"
#include "rf.h"
#include "gpio.h"


void print_rf_stats() {
    SEGGER_RTT_printf(0, "%sreq_values:%s  %d\n", RTT_CTRL_TEXT_BRIGHT_YELLOW, RTT_CTRL_TEXT_BRIGHT_GREEN, nrf_stats.req_values);
    SEGGER_RTT_printf(0, "%sres_values:%s  %d\n", RTT_CTRL_TEXT_BRIGHT_YELLOW, RTT_CTRL_TEXT_BRIGHT_GREEN, nrf_stats.res_values);
    SEGGER_RTT_printf(0, RTT_CTRL_RESET);
    char conv_buf[10];
    gcvt(((float)nrf_stats.res_values / (float)nrf_stats.req_values) * 100.0, 3, conv_buf);
    SEGGER_RTT_printf(0, "%sVal succ:%s %s%%\n", RTT_CTRL_RESET, RTT_CTRL_TEXT_BRIGHT_GREEN, conv_buf);
    SEGGER_RTT_printf(0, RTT_CTRL_RESET);
    SEGGER_RTT_printf(0, "TX OK:    %d\n", nrf_stats.tx_ok);
    SEGGER_RTT_printf(0, "TX M_RT:  %d\n", nrf_stats.tx_max_rt);
    SEGGER_RTT_printf(0, "TX T_O:   %d\n", nrf_stats.tx_timeout);
    SEGGER_RTT_printf(0, "TX succ:  %.2f %%\n", ((float)nrf_stats.tx_ok / (float)(nrf_stats.tx_ok + nrf_stats.tx_max_rt + nrf_stats.tx_timeout)) * 100.0);
}

void print_rf_status(void) {
    int s = rf_status();

    SEGGER_RTT_printf(0, "%s", "\nRF Status Register\r\n");
    SEGGER_RTT_printf(0, "%s", "RX_DR  TX_DS  MAX_RT  RX_P_NO  TX_FULL\r\n");
    SEGGER_RTT_printf(0, "%d         ", NRF_STATUS_GET_RX_DR(s));
    SEGGER_RTT_printf(0, "%d        ", NRF_STATUS_GET_TX_DS(s));
    SEGGER_RTT_printf(0, "%d       ", NRF_STATUS_GET_MAX_RT(s));
    SEGGER_RTT_printf(0, "%d       ", NRF_STATUS_GET_RX_P_NO(s));
    SEGGER_RTT_printf(0, "%d\n", NRF_STATUS_GET_TX_FULL(s));

}

void blink_led(void) {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}