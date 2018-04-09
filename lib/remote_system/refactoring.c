#include "refactoring.h"
#include "datatypes.h"
#include "packet.h"

#define NOACK               0
#define MAX_PL_LEN          25
#define RX_BUFFER_SIZE      PACKET_MAX_PL_LEN


nrf_stats_t nrf_stats;
static uint8_t rx_buffer[RX_BUFFER_SIZE];
// Don't use ack on tx (requires the same setting on the vesc too)





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



void send_command_to_vesc(COMM_PACKET_ID command) {
	uint8_t data[6];
	int32_t len = 0;

	data[len++] = command;

    send_buffer_nrf(data, len);
}




void send_buffer_nrf(unsigned char *data, unsigned int len) {

    uint8_t send_buffer[MAX_PL_LEN];


	if (data[0] == COMM_GET_VALUES) {
		nrf_stats.req_values++;
	}

    //SHORT-DATA
	if (len <= (MAX_PL_LEN - 1)) {
		// SEGGER_RTT_printf(0, "SEND send_buffer_nrf MOTE_PACKET_PROCESS_SHORT_BUFFER\n");
		uint32_t ind = 0;
		send_buffer[ind++] = MOTE_PACKET_PROCESS_SHORT_BUFFER;
		memcpy(send_buffer + ind, data, len);
		ind += len;
		rf_tx_wrapper((char*)send_buffer, ind);
	}



    
    //LONG-DATA
    // else {
	// 	unsigned int end_a = 0;
	// 	unsigned int len2 = len - (MAX_PL_LEN - 5);

	// 	for (unsigned int i = 0;i < len2;i += (MAX_PL_LEN - 2)) {
	// 		if (i > 255) {
	// 			break;
	// 		}

	// 		end_a = i + (MAX_PL_LEN - 2);

	// 		uint8_t send_len = (MAX_PL_LEN - 2);
	// 		send_buffer[0] = MOTE_PACKET_FILL_RX_BUFFER;
	// 		send_buffer[1] = i;

	// 		if ((i + (MAX_PL_LEN - 2)) <= len2) {
	// 			memcpy(send_buffer + 2, data + i, send_len);
	// 		} else {
	// 			send_len = len2 - i;
	// 			memcpy(send_buffer + 2, data + i, send_len);
	// 		}

	// 		rf_tx_wrapper((char*)send_buffer, send_len + 2);
	// 	}

	// 	for (unsigned int i = end_a;i < len2;i += (MAX_PL_LEN - 3)) {
	// 		uint8_t send_len = (MAX_PL_LEN - 3);
	// 		send_buffer[0] = MOTE_PACKET_FILL_RX_BUFFER_LONG;
	// 		send_buffer[1] = i >> 8;
	// 		send_buffer[2] = i & 0xFF;

	// 		if ((i + (MAX_PL_LEN - 3)) <= len2) {
	// 			memcpy(send_buffer + 3, data + i, send_len);
	// 		} else {
	// 			send_len = len2 - i;
	// 			memcpy(send_buffer + 3, data + i, send_len);
	// 		}

	// 		rf_tx_wrapper((char*)send_buffer, send_len + 3);
	// 	}

	// 	uint32_t ind = 0;
	// 	send_buffer[ind++] = MOTE_PACKET_PROCESS_RX_BUFFER;
	// 	send_buffer[ind++] = len >> 8;
	// 	send_buffer[ind++] = len & 0xFF;
	// 	unsigned short crc = crc16(data, len);
	// 	send_buffer[ind++] = (uint8_t)(crc >> 8);
	// 	send_buffer[ind++] = (uint8_t)(crc & 0xFF);
	// 	memcpy(send_buffer + 5, data + len2, len - len2);
	// 	ind += len - len2;

	// 	rf_tx_wrapper((char*)send_buffer, ind);
	// }
}










void process_received_vesc_data(void) {
    char buf[32];
    int len;
    int p;
    int ind;

    for(;;) {
    #if NOACK
        int res = rfhelp_read_rx_data_crc_noack(buf, &len, &p);
    #else
        int res = rfhelp_read_rx_data_crc(buf, &len, &p);
    #endif


    // If something was read
    if (res >= 0) {
        // SEGGER_RTT_printf(0, "!!!GOT DATA!!!\n");
        MOTE_PACKET packet = buf[0];



        switch (packet) {
        case MOTE_PACKET_BATT_LEVEL:
            // TODO!
            break;



        case MOTE_PACKET_ALIVE:
            // Everything triggers alive...
            SEGGER_RTT_printf(0, "GOT ALIVE PACKET!\n");
            break;



        case MOTE_PACKET_FILL_RX_BUFFER:
            memcpy(rx_buffer + buf[1], buf + 2, len - 2);
            break;



        case MOTE_PACKET_FILL_RX_BUFFER_LONG: {
            int rxbuf_ind = (unsigned int)buf[1] << 8;
            rxbuf_ind |= buf[2];
            if (rxbuf_ind < RX_BUFFER_SIZE) {
                memcpy(rx_buffer + rxbuf_ind, buf + 3, len - 3);
            }
        }
        break;




        case MOTE_PACKET_PROCESS_RX_BUFFER: {
            SEGGER_RTT_printf(0, "MOTE_PACKET_PROCESS_RX_BUFFER\n");
            ind = 1;
            int rxbuf_len = (unsigned int)buf[ind++] << 8;
            rxbuf_len |= (unsigned int)buf[ind++];

            if (rxbuf_len > RX_BUFFER_SIZE) {
                break;
            }

            uint8_t crc_high = buf[ind++];
            uint8_t crc_low = buf[ind++];

            memcpy(rx_buffer + rxbuf_len - (len - ind), buf + ind, len - ind);

            if (crc16(rx_buffer, rxbuf_len)
                    == ((unsigned short) crc_high << 8
                            | (unsigned short) crc_low)) {

                if (rx_buffer[0] == COMM_GET_VALUES) {
                    nrf_stats.res_values++;

    bldc_interface_process_packet(rx_buffer, rxbuf_len);

                }

                // Wait a bit in case retries are still made
                HAL_Delay(2);
                packet_send_packet(rx_buffer, rxbuf_len, 0);
            }
        }
        break;




            case MOTE_PACKET_PROCESS_SHORT_BUFFER:
                SEGGER_RTT_printf(0, "MOTE_PACKET_PROCESS_SHORT_BUFFER\n");
                // Wait a bit in case retries are still made
                HAL_Delay(2);
                packet_send_packet((unsigned char*)buf + 1, len - 1, 0);
                break;




            default:
                break;
            }
        }

    // Stop when there is no more data to read.
    if (res <= 0) {
        break;
    }
}

}







