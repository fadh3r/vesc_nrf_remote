#include "refactoring.h"

nrf_stats_t nrf_stats;
int nrf_restart_rx_time = 0;
int nrf_restart_tx_time = 0;

int rf_tx_wrapper(char *data, int len) {
    
    #if NOACK
        int res = rfhelp_send_data_crc_noack(data, len, TX_RESENDS);
    #else
        int res = rfhelp_send_data_crc(data, len);
    #endif

    if (res == 0) {
        nrf_stats.tx_ok++;
        nrf_restart_tx_time = NRF_RESTART_TIMEOUT;
    } else if (res == -1) {
        nrf_stats.tx_max_rt++;
    } else if (res == -2) {
        nrf_stats.tx_timeout++;
    }

    return res;
}

