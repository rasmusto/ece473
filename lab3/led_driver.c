#include "led_driver.h"


//this is simply a lookup function for the bit sequence
//that corresponds to a decimal number from 0-10
uint8_t LED_convert(uint8_t digit)
{
    if(digit == 0){
        return 0b11000000;
    }
    if(digit == 1){
        return 0b11111001;
    }
    if(digit == 2){
        return 0b10100100;
    }
    if(digit == 3){
        return 0b10110000;
    }
    if(digit == 4){
        return 0b10011001;
    }
    if(digit == 5){
        return 0b10010010;
    }
    if(digit == 6){
        return 0b10000010;
    }
    if(digit == 7){
        return 0b11111000;
    }
    if(digit == 8){
        return 0b10000000;
    }
    if(digit == 9){
        return 0b10010000;
    }
    return 0xFF;
}
