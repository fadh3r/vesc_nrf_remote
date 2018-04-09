#include "datatypes.h"
#include "mote_process_data.h"
#include "packet.h"


#define RX_BUFFER_SIZE      PACKET_MAX_PL_LEN

nrf_stats_t nrf_stats;
static uint8_t rx_buffer[RX_BUFFER_SIZE];
// Don't use ack on tx (requires the same setting on the vesc too)


void mote_process_data(void) {
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
                    }

                    // Wait a bit in case retries are still made
                    HAL_Delay(2);
                    // packet_send_packet(rx_buffer, rxbuf_len, 0);
                    bldc_interface_process_packet(rx_buffer, rxbuf_len);
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
