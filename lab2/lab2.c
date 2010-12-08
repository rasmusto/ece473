// lab2.c
// Torben Rasmussen
// 10.6.10

#define F_CPU 16000000 // cpu speed in hertz 
#define TRUE 1
#define FALSE 0
#define TRISTATE_ON_bm 0x5F //connected to pin7 of the 3-8 decoder
#define COLON_ON_bm 0b11111100
#define COLON_OFF_bm 0b11111111
#include <avr/io.h>
#include <util/delay.h>
#include "led_driver.h" //this header contains the prototypes 7-seg functions

//holds data to be sent to the segments. logic zero turns segment on
uint8_t segment_data[5];

//decimal to 7-segment LED display encodings, logic "0" turns on segment
uint8_t dec_to_7seg[12];

//******************************************************************************
//                            chk_buttons                                      
//Checks the state of the button number passed to it. It shifts in ones till   
//the button is pushed. Function returns a 1 only once per debounced button    
//push so a debounce and toggle function can be implemented at the same time.  
//Adapted to check all buttons from Ganssel's "Guide to Debouncing"            
//Expects active low pushbuttons on PINA port.  Debounce time is determined by 
//external loop delay times 12. 
//
//This code has been modified to work with a button passed in as a parameter (0-7)
uint8_t chk_buttons(uint8_t button) 
{
    static uint16_t state[8] = {0,0,0,0,0,0,0,0};
    state[button] = (state[button] << 1) | (! bit_is_clear(PINA, button)) | 0xE000;
    if (state[button] == 0xF000) return 1;
    return 0;
}
//******************************************************************************


//***********************************************************************************
//                                   segment_sum                                    
//takes a 16-bit binary input value and places the appropriate equivalent 4 digit 
//BCD segment code in the array segment_data for display.                       
//array is loaded at exit as:  |digit3|digit2|colon|digit1|digit0|
void segsum(uint16_t sum)
{
    uint8_t ones;
    uint8_t tens;
    uint8_t hundreds;
    uint8_t thousands;
    uint8_t numDigits;
    //determine how many digits there are 
    if(0 <= sum && sum < 10){
        numDigits = 1;
    }
    if(10 <= sum && sum < 100){
        numDigits = 2;
    }
    if(100 <= sum && sum < 1000){
        numDigits = 3;
    }
    if(1000 <= sum && sum < 10000){
        numDigits = 4;
    }
    //break up decimal sum into 4 digit-segments
    //these variables hold the decimal representation of each digit
    thousands = (sum / 1000) % 10;
    hundreds = (sum / 100) % 10;
    tens = (sum / 10) % 10;
    ones = (sum / 1) % 10;
    //blank out leading zero digits 
    //now move data to right place for misplaced colon position
    //this code also blanks out any leading zeros
    if(numDigits == 4){
        segment_data[4] = thousands;
        segment_data[3] = hundreds;
        segment_data[2] = COLON_OFF_bm;
        segment_data[1] = tens;
        segment_data[0] = ones;
    }
    if(numDigits == 3){
        segment_data[4] = 0xFF;
        segment_data[3] = hundreds;
        segment_data[2] = COLON_OFF_bm;
        segment_data[1] = tens;
        segment_data[0] = ones;
    }
    if(numDigits == 2){
        segment_data[4] = 0xFF;
        segment_data[3] = 0xFF;
        segment_data[2] = COLON_OFF_bm;
        segment_data[1] = tens;
        segment_data[0] = ones;
    }
    if(numDigits == 1){
        segment_data[4] = 0xFF;
        segment_data[3] = 0xFF;
        segment_data[2] = COLON_OFF_bm;
        segment_data[1] = 0xFF;
        segment_data[0] = ones;
    }
}//segment_sum
//***********************************************************************************


//***********************************************************************************
int main(void)
{   
    uint16_t count = 0;  //loop counter for updating each individual 7-seg
    uint16_t sum = 0;
    //set port bits 4-7 B as outputs
    DDRB = 0xF0;

    while(1){
        _delay_ms(2); //insert loop delay for debounce
        //make PORTA an input port with pullups 
        DDRA = 0x00; //PORTA is an input
        PORTA = 0xFF; //Enable pullups
        //enable tristate buffer for pushbutton switches
        PORTB = TRISTATE_ON_bm;
        //now check each button and increment the count as needed
		_delay_us(10);   //this delay was necessary for the first button to be read
		if (chk_buttons(0)){ sum += 128; }
        if (chk_buttons(1)){ sum += 64; }
        if (chk_buttons(2)){ sum += 32; }
        if (chk_buttons(3)){ sum += 16; }
        if (chk_buttons(4)){ sum += 8; }
        if (chk_buttons(5)){ sum += 4; }
        if (chk_buttons(6)){ sum += 2; }
        if (chk_buttons(7)){ sum += 1; }

        //bound a counter (0-4) to keep track of digit to display 
        if (count > 4)
            count = 0;
        //bound the sum to 0 - 1023
        //if (sum > 1023)
        //    sum = 0;
        sum = sum % 1024;
        //break up the disp_value to 4, BCD digits in the array: call (segsum)
        segsum(sum);
        //make PORTA an output
        DDRA = 0xFF;
        //send 7 segment code to LED segments
        PORTA = LED_convert(segment_data[count]);
        //send PORTB the digit to display
        PORTB = count << 4;  //set PORTB4-6 to select digit
        //update digit to display
        count++;
    }//while
}//main
