// lab5_m48.c
// Torben Rasmussen
// 11.16.2010

#define F_CPU 8000000   //8Mhz clock
#define BAUD 9600
#define MYUBRR 51
#include <avr/io.h>
#include <util/delay.h>
#include "/usr/lib/avr/include/avr/iom48.h" //for tons of tab completion
#include <avr/interrupt.h>
#include "twi.h"

volatile uint16_t temperature;

void USART_init(unsigned int ubrr);
volatile uint8_t high_byte_flag = 1;

int main(void)
{
    USART_init(MYUBRR);
    DDRB = 0b00000010;
    DDRC = 0b00000010;
    sei();
    UDR0 = 0x00;
    while(1){     //do forever
        if(PINB == 0x00)
            PORTB |= 0x02;
        else
            PORTB &= 0x00;
        _delay_ms(5);
        temperature = rd_temp();
    } //while 
} //main

//USART0 tx complete
ISR(USART_TX_vect)
{
    if(PINC == 0x00)
        PORTC |= 0x02;
    else
        PORTC &= 0x00;
    if(high_byte_flag)
    {
        UDR0 = temperature>>8;
        high_byte_flag = 0;
    }
    else
    {
        UDR0 = temperature;
        high_byte_flag = 1;
    }
}

void USART_init(unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    //enable tx and rx, enable tx complete interrupt
    UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<TXCIE0);
    //8 bits per character
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}
