// lab3.c
// Torben Rasmussen
// 10.26.10

#define F_CPU 16000000 // cpu speed in hertz 
#define TRUE 1
#define FALSE 0
#define TRISTATE_ON_bm 0x5F //connected to Y5 of the 3-8 decoder
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "led_driver.h"     //this header contains the prototypes 7-seg functions
//#include "/usr/lib/avr/include/avr/iom128.h" //for tons of tab completion

void segsum(uint16_t sum);
uint8_t chk_buttons(uint8_t button);
void setBar(uint8_t value);

//holds data to be sent to the segments. logic zero turns segment on
uint8_t segment_data[5];
uint16_t sum;
uint16_t count = 0;  //loop counter for updating each individual 7-seg
uint8_t mode = 1;

void tcnt0_init(void)
{
    ASSR   |=  (1<<AS0);    //ext osc TOSC
    TIMSK  |=  (1<<TOIE0);  //enable timer/counter0 overflow interrupt
    TCCR0  |=  (0<<WGM01) | (0<<WGM00) | (0<<COM00) | (0<<COM01) | (0<<CS02) | (0<<CS01) | (1<<CS00);//normal mode, no prescale
}

void spi_init(void)
{
    DDRB  |=   0b00000111;              //Turn on SS, MOSI, SCLK
    SPCR  |=   (1<<SPE) | (1<<MSTR);    //set up SPI mode
    SPSR  |=   (1<<SPI2X);              // double speed operation
}//spi_init

//this function breaks a binary uint16 into decimal digits
//for the 7-seg driver
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


//main acts to update the 7-seg displays only
int main(void)
{
    //port initialization
    DDRB = 0xFF;    //set port B as an output

    tcnt0_init();  //initalize counter timer zero
    spi_init();    //initalize SPI port
    sei();         //enable interrupts before entering loop
    sum = 0;

    while(1)
    {
        //_delay_ms(1);
        //bound a counter (0-4) to keep track of digit to display 
        if (count > 4)
            count = 0;
        //bound the sum to 0 - 1023
        sum = sum % 1024;
        //break up the disp_value to 4, BCD digits in the array: call (segsum)
        segsum(sum);
        //make PORTA an output
        DDRA = 0xFF;
        //turn off leds
        PORTA = 0xFF;
        //send PORTB the digit to display
        PORTB = count << 4;  //set PORTB4-6 to select digit
        //send 7 segment code to LED segments
        PORTA = LED_convert(segment_data[count]);
        //update digit to display
        count++;
    }
}//main

//this interrupt is triggered on a timer/counter0 overflow, and
//does 3 things:
//1) Enable tri-state buffer and check pushbuttons
//2) Read encoder data and determine which way it has been turned
//3) Update bar graph display (to show SPDR and mode)
ISR(TIMER0_OVF_vect) 
{
    static uint8_t knob1_state;
    static uint8_t knob1_old;
    static uint8_t knob2_state;
    static uint8_t knob2_old;
    static uint8_t data;

    uint8_t left_turn[4] = {3, 1, 0, 2};
    uint8_t right_turn[4] = {3, 2, 0, 1};

    static uint8_t hit_count = 4;
    uint8_t bar = 0;

    //Wiggle PB0
    PORTB = 0x01;
    PORTB = 0x00;

    //make PORTA an input port with pullups 
    DDRA = 0x00; //PORTA is an input
    PORTA = 0xFF; //Enable pullups
    //enable tristate buffer for pushbutton switches
    PORTB = TRISTATE_ON_bm;
    //now check each button and increment the count as needed
    _delay_us(10);   //this delay was necessary for the first button to be read
    //This will change the state of mode to 0b00000001, 0b00000010, 0b000000100,
    //or 0b000000111 (no change)
    if (chk_buttons(0)){mode ^= 0b00000011;}
    if (chk_buttons(1)){mode ^= 0b00000101;}
    //if (chk_buttons(2)){ setBar(0b00000100); }
    //if (chk_buttons(3)){ setBar(0b00001000); }
    //if (chk_buttons(4)){ setBar(0b00010000); }
    //if (chk_buttons(5)){ setBar(0b00100000); }
    //if (chk_buttons(6)){ setBar(0b01000000); }
    //if (chk_buttons(7)){ setBar(0b10000000); }

    data = SPDR;

    knob1_state = data & 0b00000011;
    knob2_state = data & 0b00001100;
    knob2_state = knob2_state >> 2;

    //update the mode display on the bar graph
    bar = knob1_state | (knob2_state<<2);
    switch(mode){
        case 1:
            bar |= 0b10000000;
            break;
        case 2:
            bar |= 0b01000000;
            break;
        case 4:
            bar |= 0b00100000;
            break;
        case 7:
            bar |= 0b11100000;
            break;
        default:
            break;
    }
    //set bargraph with SPDR data
    setBar(bar);
    //setBar(knob2_state);

    //If either knob has changed state, check which direction
    //it went
    uint8_t i = 0;
    
    if (knob1_state != knob1_old){
        while (left_turn[i%4] != knob1_state){
            i++;
        }
        if (knob1_old == left_turn[(i-1)%4]){
            hit_count--;
        }
    
        while (right_turn[i%4] != knob1_state){
            i++;
        }
        if (knob1_old == right_turn[(i-1)%4]){
            hit_count++;
        }
    }
    
    if (knob2_state != knob2_old){
        while (left_turn[i%4] != knob2_state){
            i++;
        }
        if (knob2_old == left_turn[(i-1)%4]){
            hit_count--;
        }
    
        while (right_turn[i%4] != knob2_state){
            i++;
        }
        if (knob2_old == right_turn[(i-1)%4]){
            hit_count++;
        }
    }

    //If the knob has turned 4 electrical ticks from where
    //it started, increment the sum and reset hit_count
    if(hit_count == 7){
        if (mode != 0x07)
            sum+=mode;
        hit_count = 4;
    }

    if(hit_count == 1)
    {
        if (mode != 0x07)
            sum-=mode;
        hit_count = 4;
    }

    //record previous state of encoders
    knob1_old = knob1_state;
    knob2_old = knob2_state;

    //disable tristate buffer
    PORTB = 0x0F;
}

//send 'value' to the bargraph display
void setBar(uint8_t value)
{
    SPDR = value; 
    while(bit_is_clear(SPSR, SPIF)){}
    PORTB = 0x60;
    PORTB = 0x00;
}
