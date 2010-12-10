#include "twi.h"

uint16_t rd_temp()
{
    TWCR = TWCR_START; //send start condition
	while (!(TWCR & (1<<TWINT))); //wait for start condition to transmit
	TWDR = LM73_READ; //send device addr, read bit set
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);//enable TWI and ACK
	while(!(TWCR & (1<<TWINT)));
	uint8_t outdoor1= TWDR;//assign byte read to variable
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);//enable TWI and ACK
	while(!(TWCR & (1<<TWINT)));
	uint8_t outdoor2 = TWDR;//assign second byte read to variable
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);//stop transmission
    return ((((outdoor1<< 8) + outdoor2) >> 5));
}
