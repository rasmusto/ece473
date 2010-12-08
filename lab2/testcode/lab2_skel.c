// lab2_skel.c 
// Gabriel Condrea
// 10/6/2010

//  HARDWARE SETUP:
//  PORTA is connected to the segments of the LED display. and to the pushbuttons.
//  PORTA.0 corresponds to segment a, PORTA.1 corresponds to segement b, etc.
//  PORTB bits 4-6 go to a,b,c inputs of the 74HC138.
//  PORTB bit 7 goes to the PWM transistor base.

#define F_CPU 16000000 // cpu speed in hertz 
#define TRUE 1
#define FALSE 0
#include <avr/io.h>
#include <util/delay.h>

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
uint8_t chk_buttons(uint8_t button) {    
    static uint16_t  state[8] = {0,0,0,0,0,0,0,0};  //holds present state  
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
void segsum(uint16_t sum) {

    uint8_t ones;
    uint8_t tens;
    uint8_t hundreds;
    uint8_t thousands;
    uint16_t numDigits; 

    //determine how many digits there are
    if(0 <= sum && sum < 10){
        numDigits = 1;
    }
    if(9 < sum && sum < 100){
        numDigits = 2;
    }
    if(99 < sum && sum < 1000){
        numDigits = 3;
    }
    if(999 < sum && sum < 10000){
        numDigits = 4;
    }
    //break up decimal sum into 4 digit-segments
    ones = sum % 10;
    tens = (sum / 10) % 10;
    hundreds = (sum/100) % 10;
    thousands = (sum/1000) % 10;

    //now move data to right place for misplaced colon position
    segment_data[0] = ones;
    segment_data[1] = tens;
    segment_data[3] = hundreds;
    segment_data[4] = thousands;

    //blank out leading zero digits
    if(numDigits == 1){
        segment_data[1] = 0xFF;
        segment_data[3] = 0xFF;
        segment_data[4] = 0xFF;
    }
    if(numDigits == 2){
        segment_data[3] = 0xFF;
        segment_data[4] = 0xFF;
    }
    if(numDigits == 3){
        segment_data[4] = 0xFF;
    }
	segment_data[2] = 0b11111111;
}//segment_sum
//***********************************************************************************

//***********************************************************************************
updateLED(uint8_t num){
    if(num == 0){
        return 0b11000000;
    }
    if(num == 1){
        return 0b11111001;
    }
    if(num == 2){
        return 0b10100100;
    }
    if(num == 3){
        return 0b10110000;
    }
    if(num == 4){
        return 0b10011001;
    }
    if(num == 5){
        return 0b10010010;
    }
    if(num == 6){
        return 0b10000010;
    }
    if(num == 7){
        return 0b11111000;
    }
    if(num == 8){
        return 0b10000000;
    }
    if(num == 9){
        return 0b10010000;
    }
    else{
        return 0b11111111;
    }
}

//***********************************************************************************

int16_t bound_count(uint16_t num, uint16_t low, uint16_t high) {
    if(num < low){
        return low;
    }
    if(num > high){
        return low;
    }
    return num;
}

//***********************************************************************************
uint8_t main()
{
    // 1 = output, 0 = input
    //set port bits 4-7 B as outputs
    DDRB = 0xF0;
    uint16_t sum = 0;
    uint16_t count;
	uint8_t one = 0b00000001;

    while(1){
        //insert loop delay for debounce
        _delay_ms(2);
        //make PORTA an input port with pullups
        DDRA = 0x00;
        PORTA = 0xFF;
        //enable tristate buffer for pushbutton switches
        PORTB = 0x7F;
        //now check each button and increment the count as needed
		if(chk_buttons(0)){
            sum = sum + one;
        }
		if(chk_buttons(1)){
            sum = sum + (one<<1);
        }
		if(chk_buttons(2)){
            sum = sum + (one<<2);
        }
		if(chk_buttons(3)){
            sum = sum + (one<<3);
        }
		if(chk_buttons(4)){
            sum = sum + (one<<4);
        }
		if(chk_buttons(5)){
            sum = sum + (one<<5);
        }
		if(chk_buttons(6)){
            sum = sum + (one<<6);
        }
		if(chk_buttons(7)){
            sum = sum + (one<<7);
        }
        //bound the count to 0 - 1023
        sum = bound_count(sum, 0, 1023);
        //break up the disp_value to 4, BCD digits in the array: call (segsum)
        segsum(sum);
        //bound a counter (0-4) to keep track of digit to display
        count = bound_count(count, 0, 4);
        //make PORTA an output 
        DDRA = 0xFF; 
        //send 7 segment code to LED segments
        PORTA = updateLED(segment_data[count]);
        //send PORTB the digit to display
        PORTB = (count << 4);
        //update digit to display
        count++;
    }//while
}//main
