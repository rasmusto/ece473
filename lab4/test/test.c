#define F_CPU 16000000 //16Mhz clock
#include <avr/io.h>

int main(void)
{
    DDRF    = 0b00000001;
    PORTF   = 0b00000001;
    while(1) {}
}
