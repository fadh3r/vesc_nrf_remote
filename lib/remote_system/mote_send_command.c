#include "mote_process_data.h"
#include "datatypes.h"



#define NOACK               0
#define MAX_PL_LEN          25



void send_buffer_nrf(unsigned char *data, unsigned int len) {

    uint8_t send_buffer[MAX_PL_LEN];

	if (data[0] == COMM_GET_VALUES) {
		nrf_stats.req_values++;
	}


    //SEND-SHORT-DATA
	if (len <= (MAX_PL_LEN - 1)) {
		// SEGGER_RTT_printf(0, "SEND send_buffer_nrf MOTE_PACKET_PROCESS_SHORT_BUFFER\n");
		uint32_t ind = 0;
		send_buffer[ind++] = MOTE_PACKET_PROCESS_SHORT_BUFFER;
		memcpy(send_buffer + ind, data, len);
		ind += len;
		rf_tx_wrapper((char*)send_buffer, ind);
	}



    
    //SEND-LONG-DATA
    else {
		unsigned int end_a = 0;
		unsigned int len2 = len - (MAX_PL_LEN - 5);

		for (unsigned int i = 0;i < len2;i += (MAX_PL_LEN - 2)) {
			if (i > 255) {
				break;
			}

			end_a = i + (MAX_PL_LEN - 2);

			uint8_t send_len = (MAX_PL_LEN - 2);
			send_buffer[0] = MOTE_PACKET_FILL_RX_BUFFER;
			send_buffer[1] = i;

			if ((i + (MAX_PL_LEN - 2)) <= len2) {
				memcpy(send_buffer + 2, data + i, send_len);
			} else {
				send_len = len2 - i;
				memcpy(send_buffer + 2, data + i, send_len);
			}

			rf_tx_wrapper((char*)send_buffer, send_len + 2);
		}

		for (unsigned int i = end_a;i < len2;i += (MAX_PL_LEN - 3)) {
			uint8_t send_len = (MAX_PL_LEN - 3);
			send_buffer[0] = MOTE_PACKET_FILL_RX_BUFFER_LONG;
			send_buffer[1] = i >> 8;
			send_buffer[2] = i & 0xFF;

			if ((i + (MAX_PL_LEN - 3)) <= len2) {
				memcpy(send_buffer + 3, data + i, send_len);
			} else {
				send_len = len2 - i;
				memcpy(send_buffer + 3, data + i, send_len);
			}

			rf_tx_wrapper((char*)send_buffer, send_len + 3);
		}

		uint32_t ind = 0;
		send_buffer[ind++] = MOTE_PACKET_PROCESS_RX_BUFFER;
		send_buffer[ind++] = len >> 8;
		send_buffer[ind++] = len & 0xFF;
		unsigned short crc = crc16(data, len);
		send_buffer[ind++] = (uint8_t)(crc >> 8);
		send_buffer[ind++] = (uint8_t)(crc & 0xFF);
		memcpy(send_buffer + 5, data + len2, len - len2);
		ind += len - len2;

		rf_tx_wrapper((char*)send_buffer, ind);
	}
}



int rf_tx_wrapper(char *data, int len) {
    
    #if NOACK
        int res = rfhelp_send_data_crc_noack(data, len, TX_RESENDS);
    #else
        int res = rfhelp_send_data_crc(data, len);
    #endif

    if (res == 0) {
        nrf_stats.tx_ok++;
    } else if (res == -1) {
        nrf_stats.tx_max_rt++;
    } else if (res == -2) {
        nrf_stats.tx_timeout++;
    }

    return res;
}




