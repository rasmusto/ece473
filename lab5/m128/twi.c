#include "twi.h"
#include "lcd.h"

uint16_t rd_temp()
{
    /*
    TWCR = TWCR_START;           //send start condition

    TWDR = LM73_READ;            //send device addr, read bit set
    TWCR = TWCR_RACK;                 //receive data byte, return ACK

    while(!(TWCR & (1<<TWINT))){}     //wait for data byte to come in
    if(TW_STATUS != TW_MR_DATA_ACK){return(1);}  //byte 1 read failure  
    int lm73_temp_high = TWDR;            //store temp high byte 

    TWCR = TWCR_RNACK;                //recv temp low byte, return NACK 
    while(!(TWCR & (1<<TWINT))){}     //wait for data byte to come in 
    if(TW_STATUS != TW_MR_DATA_NACK){return(2);} //byte 2 read failure
    int lm73_temp_low = TWDR;             //store temp low byte 
    TWCR = TWCR_STOP;                 //conclude transaction
    return(lm73_temp_low + (lm73_temp_high<<8));                        //return success value
    */
    TWCR = TWCR_START; //send start condition
	while (!(TWCR & (1<<TWINT))); //wait for start condition to transmit
	if((TWSR & 0xF8) != TW_START){
		string2lcd("ERROR1"); //check start status
	}
	TWDR = LM73_READ; //send device addr, read bit set
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if((TWSR & 0xf8) != TW_MR_SLA_ACK){
		string2lcd("ERROR2");
	}
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);//enable TWI and ACK
	while(!(TWCR & (1<<TWINT)));
	uint8_t indoor1= TWDR;//assign byte read to variable
	if((TWSR & 0xF8) != TW_MR_DATA_ACK){
	}
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);//enable TWI and ACK
	while(!(TWCR & (1<<TWINT)));
	uint8_t indoor2 = TWDR;//assign second byte read to variable
	if((TWSR & 0xF8) != TW_MR_DATA_ACK){
	}
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);//stop transmission
    return ((((indoor1 << 8) + indoor2) >> 5));
}   
