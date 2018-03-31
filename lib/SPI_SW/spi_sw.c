/*
	Copyright 2015 Benjamin Vedder	benjamin@vedder.se

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#include "spi_sw.h"
// #include "ch.h"
// #include "hal.h"
#include <stdbool.h>
#include "gpio.h"


///MY-IMPLEMENTATIONS-OF-CHIBI-OS-FUNCTIONS
void palSetPadMode(port, pin, mode) {

// Mode
// The mode of the selected pins (Input / Output / etc.)
// - e.g. GPIO_MODE_INPUT
// - Possible assignments are the following:
// GPIO_MODE_INPUT floating input
// GPIO_MODE_OUTPUT_PP output push-pull
// GPIO_MODE_OUTPUT_OD output open drain
// GPIO_MODE_AF_PP alternate function output push-pull
// GPIO_MODE_AF_OD alternate function output open drain 
// GPIO_MODE_AF_INPUT alternate function input 
//
// Pull
// - Pull-up or Pull-down resistors for the specified pins.
// - Can be the following:
// GPIO_NOPULL
// GPIO_PULLUP
// GPIO_PULLDOWN 
//
// Speed
// - Specifies the speed for the selected pins
// - Can be the following:
// GPIO_SPEED_LOW
// GPIO_SPEED_MEDIUM
// GPIO_SPEED_HIGH 

	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = mode;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	if (pin == PIN_MISO ) {
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		// GPIO_InitStruct.Pull = GPIO_PULLUP;
		}

	HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void palSetPad(port, pin) {//Sets a pad logical state to PAL_HIGH
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

void palClearPad(port, pin) {
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

void palWritePad(port, pin, state) {
	HAL_GPIO_WritePin(port, pin, state);
}

int palReadPad(port, pad) {
	return HAL_GPIO_ReadPin(port, pad);
}







// Private functions
static void spi_sw_delay(void);

// Private variables
static bool init_done = false;

void spi_sw_init(void) {
	if (!init_done) {
		palSetPadMode(PORT_MISO, PIN_MISO, GPIO_MODE_INPUT);

		palSetPadMode(PORT_CE, PIN_CE, GPIO_MODE_OUTPUT_PP);
		palSetPadMode(PORT_CSN, PIN_CSN, GPIO_MODE_OUTPUT_PP);
		
		palSetPadMode(PORT_SCK, PIN_SCK, GPIO_MODE_OUTPUT_PP);
		palSetPadMode(PORT_MOSI, PIN_MOSI, GPIO_MODE_OUTPUT_PP);

		palSetPad(PORT_CSN, PIN_CSN);
		palClearPad(PORT_SCK, PIN_SCK);
		init_done = true;
	}
}

void spi_sw_transfer(char *in_buf, const char *out_buf, int length) {
	palClearPad(PORT_SCK, PIN_SCK);

	for (int i = 0;i < length;i++) {
		unsigned char send = out_buf ? out_buf[i] : 0;
		unsigned char recieve = 0;

		for (int bit=0;bit < 8;bit++) {
			palWritePad(PORT_MOSI, PIN_MOSI, send >> 7);
			send <<= 1;

			spi_sw_delay();

			recieve <<= 1;
			if (palReadPad(PORT_MISO, PIN_MISO)) {
				recieve |= 0x1;
			}

			palSetPad(PORT_SCK, PIN_SCK);
			spi_sw_delay();
			palClearPad(PORT_SCK, PIN_SCK);
		}

		if (in_buf) {
			in_buf[i] = recieve;
		}
	}
}

void spi_sw_begin(void) {
	palClearPad(PORT_CSN, PIN_CSN);
}

void spi_sw_end(void) {
	palSetPad(PORT_CSN, PIN_CSN);
}

void spi_sw_write_ce(int state) {
	palWritePad(PORT_CE, PIN_CE, state);
}

static void spi_sw_delay(void) {
	__NOP();
	// Possibly more NOPs on a faster MCU
}