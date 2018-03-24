//MCU STM32 page -> http://www.st.com/en/microcontrollers/stm32f103c8.html
#include "main.h"
#include "stm32f1xx_hal.h" //подключает весь пакет HAL
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#include "gpio.h"
#include "rf.h"
#include "i2c.h"
#include "dma.h"
#include "tim.h"
#include "ssd1306.h"
#include "fonts.h"

#include "datatypes.h"
#include "bldc_interface.h"
#include "conf_general.h"
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "spi_sw.h"
#include "rf.h"
#include "rfhelp.h"
#include "buffer.h"
#include "packet.h"
#include "crc.h"
#include <stdarg.h>
/**
 * The STM32 factory-programmed UUID memory.
 * Three values of 32 bits each starting at this address
 * Use like this: STM32_UUID[0], STM32_UUID[1], STM32_UUID[2]
 */
#define STM32_UUID ((uint32_t *)0x1FFFF7E8)
#define STM32_UUID_8 ((uint8_t *)0x1FFFF7E8)

#define ALIVE_TIMEOUT_MS		10000

#define MAX_PL_LEN				25
#define RX_BUFFER_SIZE			PACKET_MAX_PL_LEN
#define SERIAL_RX_BUFFER_SIZE	1024

#define USE_PRINTF				1    // 0 = off, 1 = direct, 2 = BLDC Tool
#define PRINT_MAIN				0    // Print some stats
#define PRINT_NRF_STATS			1    // Print NRF packet stats
#define TX_DISABLE_TIME			200  // Disable the chuck packets for this time when the uart bridge is used
#define RX_ENABLE_TIME			200  // Keep RX on for at least this time when the uart bridge is used
#define NRF_RESTART_TIMEOUT		500  // Restart the NRF if nothing has been received or acked for this time

// Don't use ack on tx (requires the same setting on the vesc too)
#define NOACK					0
#define TX_RESENDS				1

#define CR_DS_MASK				((uint32_t)0xFFFFFFFC)
#define PWR_Regulator_ON		((uint32_t)0x00000000)
#define PWR_Regulator_LowPower	((uint32_t)0x00000001)



//----------refactored--------------
#include "ss495a.h"
//----------end-of-refactored--------------







// Datatypes
typedef struct {
	uint8_t js_x;
	uint8_t js_y;
	bool bt_c;
	bool bt_z;
	bool bt_push;
	float vin;
	float vbat;
} mote_state;

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



// Variables
// static mutex_t print_mutex;
// static mutex_t read_mutex;
// static THD_WORKING_AREA(rx_thread_wa, 256);
// static THD_WORKING_AREA(tx_thread_wa, 256);
// static THD_WORKING_AREA(packet_process_thread_wa, 256);
// static adcsample_t adc_samples[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
// static systime_t alive_timestamp = 0;
// static thread_t *rx_tp;
// static thread_t *tx_tp;
// static thread_t *process_tp;
static bool rx_thd_is_waiting = false;
static bool tx_thd_is_waiting = false;
static bool rx_now = false;
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint8_t serial_rx_buffer[SERIAL_RX_BUFFER_SIZE];
static int serial_rx_read_pos = 0;
static int serial_rx_write_pos = 0;
static int tx_disable_time = 0;
static int rx_enable_time = 0;
static nrf_stats_t nrf_stats;
static int nrf_restart_rx_time = 0;
static int nrf_restart_tx_time = 0;
static char radio_address[3];
static unsigned char radio_channel;
static const char radio_address_pairing[3] = {0xC6, 0xC5, 0};
static const unsigned char radio_channel_pairing = 124;

// Functions
void printf_thd(const char* format, ...);
static void print_rf_status(void);
static int rf_tx_wrapper(char *data, int len);
static void read_mote_state(mote_state *data);
static void send_buffer_nrf(unsigned char *data, unsigned int len);
static uint32_t crc32c(uint8_t *data, uint32_t len);




print_rf_current_address() {
	SEGGER_RTT_printf(0, "Address0: %d\n", radio_address[0]);
	SEGGER_RTT_printf(0, "Address1: %d\n", radio_address[1]);
	SEGGER_RTT_printf(0, "Address2: %d\n",radio_address[2]);
	SEGGER_RTT_printf(0, "Channel: %d\n", radio_channel);
	// SEGGER_RTT_printf(0, "STM32_UUID Address0: %d\n", radio_address[0]);
    // SEGGER_RTT_printf(0, "STM32_UUID Address1: %d\n", radio_address[1]);
    // SEGGER_RTT_printf(0, "STM32_UUID Address2: %d\n",radio_address[2]);
    // SEGGER_RTT_printf(0, "STM32_UUID Channel: %d\n", radio_channel);
}

print_rf_stats() {
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

static void print_rf_status(void) {
	int s = rf_status();

	SEGGER_RTT_printf(0, "%s", "\nRF Status Register\r\n");
	SEGGER_RTT_printf(0, "%s", "RX_DR  TX_DS  MAX_RT  RX_P_NO  TX_FULL\r\n");
	SEGGER_RTT_printf(0, "%d         ", NRF_STATUS_GET_RX_DR(s));
	SEGGER_RTT_printf(0, "%d        ", NRF_STATUS_GET_TX_DS(s));
	SEGGER_RTT_printf(0, "%d       ", NRF_STATUS_GET_MAX_RT(s));
	SEGGER_RTT_printf(0, "%d       ", NRF_STATUS_GET_RX_P_NO(s));
	SEGGER_RTT_printf(0, "%d\n", NRF_STATUS_GET_TX_FULL(s));
	
}

static int rf_tx_wrapper(char *data, int len) {
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





static void send_packet(unsigned char *data, unsigned int len) {
	// Wait for the previous transmission to finish.
	// while (UARTD1.txstate == UART_TX_ACTIVE) {
	// 	chThdSleep(1);
	// }

	// uartStartSend(&UARTD1, len, data);
}






static uint32_t crc32c(uint8_t *data, uint32_t len) {
	uint32_t crc = 0xFFFFFFFF;

	for (uint32_t i = 0; i < len;i++) {
		uint32_t byte = data[i];
		crc = crc ^ byte;

		for (int j = 7;j >= 0;j--) {
			uint32_t mask = -(crc & 1);
			crc = (crc >> 1) ^ (0x82F63B78 & mask);
		}
	}

	return ~crc;
}


//DISPLAY-DOX-PART-START
//https://fontforge.github.io/en-US/documentation/


//BITMAP-FONTS
//https://thenextweb.com/dd/2012/01/31/get-your-bitmap-on-with-these-11-surprisingly-good-pixel-typefaces/
//https://github.com/Tecate/bitmap-fonts
//http://www.webdesigndev.com/pixel-fonts-bitmap-fonts/



//NRF-NA-RUSSKOM
//http://blog.sci-smart.ru/2013/06/nrf24l01.html
//https://github.com/wendlers/libemb/tree/master/libnrf24l01
//https://github.com/nt4t/Chibios-STM32F103-nRF24l01-spi-server-sb
//http://www.cyberforum.ru/arm/thread2091410.html
//https://github.com/petoknm/NRF24L01
//https://github.com/LonelyWolf/stm32/tree/master/nrf24l01

//!!!!!!!!!!!!!!!!!!!!!!!!!!!
//https://github.com/vedderb/nrf24l01plus_stm32f100_chibios
//https://github.com/vedderb/nrf24l01plus_stm32f100_chibios
//https://github.com/vedderb/nrf24l01plus_stm32f100_chibios





//https://github.com/FortAwesome/Font-Awesome/tree/master/advanced-options/raw-svg/brands


//https://habrahabr.ru/post/313490/
//https://github.com/SL-RU/sdmplayer/tree/master/src/v002/Hardware

//https://bitbucket.org/nadyrshin_ryu/ssd1306_stm32/src

//https://github.com/galpavlin/STM32-SSD1306/tree/master/128x64


//http://forum.easyelectronics.ru/viewtopic.php?f=35&t=14117

//DISPLAY-DOX-PART-END



// JLinkRTTLogger -device STM32F103C8 -RTTChannel 0 -if swd -speed 4000 /dev/stdout
// dont forget copy linker script from cubemx to
//cp .platfromio/packages/framework-stm32cube/platformio/ldscripts/STM32F103CI_FLASH.ld



//http://www.electric-skateboard.builders/t/vesc-nunchuk-rf/588/30
//https://github.com/vedderb/nunchuk_mod
//http://www.electric-skateboard.builders/t/vesc-nunchuk-rf/588/147
//http://vedder.se/forums/viewtopic.php?f=6&t=248&sid=4975f8e1079a2108e1944f30dfafb2bb&start=10


/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
I2C_HandleTypeDef hi2c1;
ADC_HandleTypeDef hadc1;

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_USART1_UART_Init(void);
/* Private function prototypes -----------------------------------------------*/




ADC_Data = 0;




int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
//   MX_SPI1_Init();

//----------refactored--------------
ss495a_init();
//----------end-of-refactored--------------



SSD1306_Init();
HAL_Delay(1000);

SSD1306_GotoXY(0, 6); //Устанавливаем курсор в позицию 0;44. Сначала по горизонтали, потом вертикали.
SSD1306_Puts("Hello! 12345AaBbCc", &Font_8x13, SSD1306_COLOR_WHITE); //пишем надпись в выставленной позиции шрифтом "Font_7x10" белым цветом. 
SSD1306_DrawCircle(120, 10, 7, SSD1306_COLOR_WHITE); //рисуем белую окружность в позиции 10;33 и радиусом 7 пикселей
SSD1306_UpdateScreen();








void send_buffer_nrf(unsigned char *data, unsigned int len) {
	uint8_t send_buffer[MAX_PL_LEN];

	// tx_disable_time = TX_DISABLE_TIME;
	// rx_enable_time = RX_ENABLE_TIME;

	if (data[0] == COMM_GET_VALUES) {
		nrf_stats.req_values++;
	}

	if (len <= (MAX_PL_LEN - 1)) {
		// SEGGER_RTT_printf(0, "SEND send_buffer_nrf MOTE_PACKET_PROCESS_SHORT_BUFFER\n");
		uint32_t ind = 0;
		send_buffer[ind++] = MOTE_PACKET_PROCESS_SHORT_BUFFER;
		memcpy(send_buffer + ind, data, len);
		ind += len;
		rf_tx_wrapper((char*)send_buffer, ind);
	} else {
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









	// Packet interface
	packet_init(send_packet, send_buffer_nrf, 0);
	
	// NRF
	rf_init();
	rfhelp_init();
	print_rf_status();



	
	// Set RF address and channel based on the hashed UUID,
	// The reason for hashing is than only a few numbers in the UUID seem to be
	// updated between different chips:
	// http://false.ekta.is/2012/06/stm32-device-electronic-signature-unique-device-id-register/
	uint32_t h = crc32c(STM32_UUID_8, 12);

    radio_address[0] = h & 0xFF;
    radio_address[1] = (h >> 8) & 0xFF;
    radio_address[2] = (h >> 16) & 0xFF;
    radio_channel = (h >> 24) & 0x7F;

    if (radio_channel <= 5) {
    	radio_channel += 5;
    }

    if (radio_channel >= 120) {
    	radio_channel -= 10;
    }



	rfhelp_set_rx_addr(0, radio_address, 3);
	rfhelp_set_tx_addr(radio_address, 3);
	rfhelp_set_radio_channel(radio_channel);





	// Restart radio
	HAL_Delay(5);
	rfhelp_power_up();
	HAL_Delay(1);
	rfhelp_restart();
	


void rf_pair_and_set() {
				// Send out pairing information
				rfhelp_set_rx_addr(0, radio_address_pairing, 3);
				rfhelp_set_tx_addr(radio_address_pairing, 3);
				rfhelp_set_radio_channel(radio_channel_pairing);

				uint8_t pl[6];
				int32_t index = 0;
				pl[index++] = MOTE_PACKET_PAIRING_INFO;
				pl[index++] = (uint8_t)radio_address[0];
				pl[index++] = (uint8_t)radio_address[1];
				pl[index++] = (uint8_t)radio_address[2];
				pl[index++] = radio_channel;

				rfhelp_power_up();
				HAL_Delay(2);
				rf_tx_wrapper((char*)pl, index);

				rfhelp_set_rx_addr(0, radio_address, 3);
				rfhelp_set_tx_addr(radio_address, 3);
				rfhelp_set_radio_channel(radio_channel);

		HAL_Delay(5);

	print_rf_status();
}

rf_pair_and_set();




  int i = 0;
  uint8_t receiveBuffer[16] = { 0 };
  while (1) {

    // HAL_UART_Receive(&huart1, receiveBuffer, 16, 1024);
    // HAL_UART_Transmit(&huart1, "hello\n", 6, 1024);

    // SEGGER_RTT_WriteString(0, receiveBuffer);
    // SEGGER_RTT_WriteString(0, "\n");
    // SEGGER_RTT_printf(0, "%d\r\n", i);
    // //http://forum.segger.com/index.php?page=Thread&threadID=2881


    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_Delay(100);
    i+=1;

 
//   SSD1306_GotoXY(5, 20); //Устанавливаем курсор в позицию 0;44. Сначала по горизонтали, потом вертикали.
//   SSD1306_Puts(receiveBuffer, &Font_8x13, SSD1306_COLOR_WHITE); //пишем надпись в выставленной позиции шрифтом "Font_7x10" белым цветом. 
//   SSD1306_UpdateScreen();

















	uint8_t pl[6];
	int32_t index = 0;
	pl[index++] = COMM_GET_VALUES;
	send_buffer_nrf(pl, index);

	// index = 0;
	// pl[index++] = MOTE_PACKET_PROCESS_SHORT_BUFFER;
	// pl[index++] = COMM_GET_VALUES;

	// rfhelp_power_up();
	// HAL_Delay(2);
	// rf_tx_wrapper((char*)pl, index);


	HAL_Delay(10);



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
				nrf_restart_rx_time = NRF_RESTART_TIMEOUT;

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

	// HAL_Delay(10);






	print_rf_stats();
	print_rf_status();



char hall_value_conv_buffer[30] = { 0 };
itoa(ADC_Data* 100 / 4096, hall_value_conv_buffer, 10); // ADC_Data * 100 / 4096 при 12 битах всего 4096 вариантов %
if(strlen(hall_value_conv_buffer) < 4) {
	strcat(hall_value_conv_buffer, " ");
}
// SEGGER_RTT_printf(0, "%sadc values%s\n", RTT_CTRL_TEXT_BRIGHT_RED, RTT_CTRL_RESET);
// SEGGER_RTT_printf(0, "%s\n", ADC_Data);
SSD1306_GotoXY(20, 45); //Устанавливаем курсор в позицию 0;44. Сначала по горизонтали, потом вертикали.
SSD1306_Puts(hall_value_conv_buffer, &font_terminus_x20b, SSD1306_COLOR_WHITE); //пишем надпись в выставленной позиции шрифтом "Font_7x10" белым цветом. 
SSD1306_UpdateScreen();	










}//end-of-while(1)








}//end-of-main()





/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
