#include "refactoring.h"
#include "datatypes.h"
#include "packet.h"
#include "bldc_interface.h"
#include "bldc_interface_uart.h"
#include "ssd1306.h"
#include "SEGGER_RTT.h"




void callback_process_received_data() {

		char conv_buffer[10];
		values.v_in -= .2; //погрешность
		gcvt(values.v_in, 4, conv_buffer);
		if(strlen(conv_buffer) == 2) {
			strcat(conv_buffer, ".0");
		}
		SSD1306_GotoXY(20, 20);
		SSD1306_Puts(conv_buffer, &font_terminus_x32b, SSD1306_COLOR_WHITE);
		//4 символа * на ширину 16 = 64, 64 + 20 = 84
		//
		//высота начала 20 + 32 высота предыдущего шрифта = 52 низ предыдущего шрифта
		//52 - высота текущего шрифта 18 = 34
		//
		SSD1306_GotoXY(84, 31);
		SSD1306_Puts("v", &font_terminus_x18b, SSD1306_COLOR_WHITE);
		// SSD1306_UpdateScreen();	
		SEGGER_RTT_printf(0, RTT_CTRL_TEXT_BRIGHT_RED"VOLTAGE: %s\n"RTT_CTRL_RESET, conv_buffer);


}
