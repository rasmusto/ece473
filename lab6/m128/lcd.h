//headers for LCD functions 
//Roger Traylor 4.26.07

//#define F_CPU 16000000UL //16Mhz clock
#include <avr/interrupt.h>

void strobe_lcd(void);
void clear_display(void);
void cursor_home(void);
void home_line2(void);      
void fill_spaces(void);
void char2lcd(char a_char);
void string2lcd(char *lcd_str);
void lcd_init(void);
