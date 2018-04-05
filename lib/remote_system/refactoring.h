

#include "stdint.h"

#define NRF_RESTART_TIMEOUT		500  // Restart the NRF if nothing has been received or acked for this time

typedef struct {
	uint32_t req_values;
	uint32_t res_values;
	uint32_t tx_ok;
	uint32_t tx_max_rt;
	uint32_t tx_timeout;
} nrf_stats_t;

typedef enum {
	MOTE_PACKET_BATT_LEVEL = 0,
	MOTE_PACKET_BUTTONS,
	MOTE_PACKET_ALIVE,
	MOTE_PACKET_FILL_RX_BUFFER,
	MOTE_PACKET_FILL_RX_BUFFER_LONG,
	MOTE_PACKET_PROCESS_RX_BUFFER,
	MOTE_PACKET_PROCESS_SHORT_BUFFER,
	MOTE_PACKET_PAIRING_INFO
} MOTE_PACKET;

extern nrf_stats_t nrf_stats;
extern int nrf_restart_rx_time;
extern int nrf_restart_tx_time;

int rf_tx_wrapper(char *data, int len);