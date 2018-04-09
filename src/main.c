#include "main.h"
#include "refactoring.h"
//----------refactored--------------
#include "ss495a.h"
#include "pairing.h"
#include "mote_display.h"
#include "debug_feedback.h"

//----------end-of-refactored--------------


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
#include "icons.h"

#include "stm32f1xx_it.h"
#include "init.h"


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






// Functions


static void send_packet(unsigned char *data, unsigned int len) {
	// Wait for the previous transmission to finish.
	// while (UARTD1.txstate == UART_TX_ACTIVE) {
	// 	chThdSleep(1);
	// }

	// uartStartSend(&UARTD1, len, data);
}



UART_HandleTypeDef huart1;
/* Private function prototypes -----------------------------------------------*/
static void MX_USART1_UART_Init(void);
/* Private function prototypes -----------------------------------------------*/


volatile uint32_t SS495_ADC_buffer = 0;;  //сделать 16битной












int main(void) {
    stm32_peripherals_init();
    mote_init();
    // show_startup_logo();


    // uint8_t receiveBuffer[16] = { 0 };
    // HAL_UART_Receive(&huart1, receiveBuffer, 16, 1024);
    // HAL_UART_Transmit(&huart1, "hello\n", 6, 1024);
    // SEGGER_RTT_WriteString(0, receiveBuffer);
    // SEGGER_RTT_WriteString(0, "\n");
    // SEGGER_RTT_printf(0, "%d\r\n", i);

    while (1) {

        //get hall value and send it to vesc

        send_command_to_vesc(COMM_GET_VALUES);
        
        //WAIT FOR ANSWER
        HAL_Delay(12);
        
        process_received_vesc_data();

        print_rf_stats();
        print_rf_status();


        char hall_value_conv_buffer[30] = { 0 };
        itoa(SS495_ADC_buffer * 100 / 4096, hall_value_conv_buffer, 10); // SS495_ADC_buffer[512] * 100 / 4096 при 12 битах всего 4096 вариантов %

        if(strlen(hall_value_conv_buffer) < 4) {
            strcat(hall_value_conv_buffer, " ");
        }

        SSD1306_GotoXY(20, 45);
        SSD1306_Puts(hall_value_conv_buffer, &font_terminus_x20b, SSD1306_COLOR_WHITE);
        SSD1306_UpdateScreen();	


    }//end-of-while(1)

}//end-of-main()


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) { }
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
