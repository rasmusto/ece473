// lab1_code.c 
// R. Traylor
// 7.21.08

//This program increments a binary display of the number of button pushes on switch 
//S0 on the mega128 board.

#define F_CPU 16000000UL     //16Mhz clock
#include <avr/io.h>
#include <util/delay.h>

//*******************************************************************************
//                            debounce_switch                                  
// Adapted from Ganssel's "Guide to Debouncing"            
// Checks the state of pushbutton S0 It shifts in ones till the button is pushed. 
// Function returns a 1 only once per debounced button push so a debounce and toggle 
// function can be implemented at the same time.  Expects active low pushbutton on 
// Port D bit zero.  Debounce time is determined by external loop delay times 12. 
//*******************************************************************************
int8_t debounce_switch() {
    static uint16_t state = 0; //holds present state
    state = (state << 1) | (! bit_is_clear(PIND, 0)) | 0xE000;
    if (state == 0xF000) return 1;
    return 0;
}

//*******************************************************************************
// Check switch S0.  When found low for 12 passes of "debounce_switch(), increment
// PORTB.  This will make an incrementing count on the port B LEDS. (Now in BCD)
//*******************************************************************************
int main()
{
    uint8_t tens = 0;
    uint8_t ones = 0;
    DDRB = 0xFF;  //set port B to all outputs
    while(1){     //do forever
        if(debounce_switch()) 
        {
            if(ones < 9)
            {
                ones++;
                PORTB = ones | (tens << 4);
            }
            else
            {
                if(tens < 10){
                    tens++;
                }
                if(ones == 9 && tens == 10)
                {
                    tens = 0;
                }
                ones = 0;
                PORTB = ones | (tens << 4);
            }
        }
        _delay_ms(2);                    //keep in loop to debounce 24ms
    } //while 
} //main
