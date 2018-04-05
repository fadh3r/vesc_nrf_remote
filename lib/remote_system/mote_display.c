#include "mote_display.h"
#include "ssd1306.h"
#include "fonts.h"
#include "icons.h"


void show_startup_logo() {
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	ssd1306_image(open_source_logo, 0, 0, 1);
	SSD1306_DrawRectangle(0, 0, 128, 64, SSD1306_COLOR_WHITE);

	SSD1306_GotoXY(20, 48);
	SSD1306_Puts("open", &font_terminus_x14b, SSD1306_COLOR_WHITE); 
	SSD1306_GotoXY(61, 48);
	SSD1306_Puts("source", &font_terminus_x14b, SSD1306_COLOR_WHITE); 
	SSD1306_UpdateScreen();

	HAL_Delay(500);

	SSD1306_GotoXY(8, 48);
	SSD1306_Puts("  initiative  ", &font_terminus_x14b, SSD1306_COLOR_WHITE); 
	SSD1306_UpdateScreen();
	HAL_Delay(500);
	SSD1306_Fill(SSD1306_COLOR_BLACK);
}
