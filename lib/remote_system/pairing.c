#include "pairing.h"
#include "crc.h"
#include "rfhelp.h"
#include "refactoring.h"

/**
 * The STM32 factory-programmed UUID memory.
 * Three values of 32 bits each starting at this address
 * Use like this: STM32_UUID[0], STM32_UUID[1], STM32_UUID[2]
 */
#define STM32_UUID ((uint32_t *)0x1FFFF7E8)
#define STM32_UUID_8 ((uint8_t *)0x1FFFF7E8)

void rf_pair_and_set() {
    unsigned char unique_radio_channel;
    char unique_radio_address[3];

    const char radio_address_pairing[3] = { 0xC6, 0xC5, 0 };
    const unsigned char radio_channel_pairing = 124;

    // Set RF address and channel based on the hashed UUID,
    // The reason for hashing is than only a few numbers in the UUID seem to be
    // updated between different chips:
    // http://false.ekta.is/2012/06/stm32-device-electronic-signature-unique-device-id-register/
    uint32_t h = crc32c(STM32_UUID_8, 12);

    unique_radio_address[0] = h & 0xFF;
    unique_radio_address[1] = (h >> 8) & 0xFF;
    unique_radio_address[2] = (h >> 16) & 0xFF;
    unique_radio_channel = (h >> 24) & 0x7F;

    if (unique_radio_channel <= 5) unique_radio_channel += 5;

    if (unique_radio_channel >= 120) unique_radio_channel -= 10;

    // Send out pairing information
    rfhelp_set_rx_addr(0, radio_address_pairing, 3);
    rfhelp_set_tx_addr(radio_address_pairing, 3);
    rfhelp_set_radio_channel(radio_channel_pairing);



    //Send out address and chanel based on current STM32 UUID
    uint8_t payload[6];
    int32_t index = 0;
    payload[index++] = MOTE_PACKET_PAIRING_INFO;
    payload[index++] = (uint8_t)unique_radio_address[0];
    payload[index++] = (uint8_t)unique_radio_address[1];
    payload[index++] = (uint8_t)unique_radio_address[2];
    payload[index++] = unique_radio_channel;
    rfhelp_power_up();
    HAL_Delay(2);
    rf_tx_wrapper((char*)payload, index);


				rfhelp_set_rx_addr(0, unique_radio_address, 3);
				rfhelp_set_tx_addr(unique_radio_address, 3);
				rfhelp_set_radio_channel(unique_radio_channel);

		HAL_Delay(5);

	// print_rf_status();
}
