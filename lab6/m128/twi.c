#include "twi.h"
#include "lcd.h"

uint16_t rd_temp()
{
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
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);//enable TWI and ACK
    while(!(TWCR & (1<<TWINT)));
    uint8_t indoor2 = TWDR;//assign second byte read to variable
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);//stop transmission
    return ((((indoor1 << 8) + indoor2) >> 5));
}   

uint16_t send_radio(uint16_t station, uint8_t mute)
{
    uint16_t temp;
    temp = (4 * ((station * 100000) + 225000)) / 32768;

    TWCR = TWCR_START; //send start condition
    while(!(TWCR & (1<<TWINT))){} //wait for start condition to transmit

    TWDR = RADIO_ADDRESS;
    //send device addr, write bit set
    TWCR = TWCR_SEND;
    //poke TWINT to send address
    while(!(TWCR & (1<<TWINT))){}
    //wait for radio address to go out

    TWDR = (temp >> 8) | (mute << 7);
    //send pointer address to radio
    TWCR = TWCR_SEND;
    //poke TWINT to send address
    while(!(TWCR & (1<<TWINT))){}
    //wait for radio data byte 1 to go out

    TWDR = temp;
    //send pointer address to radio
    TWCR = TWCR_SEND;
    //poke TWINT to send address
    while(!(TWCR & (1<<TWINT))){}
    //wait for radio data byte 1 to go out

    TWDR = RADIO_BYTE_3;
    //send pointer address to radio
    TWCR = TWCR_SEND;
    //poke TWINT to send address
    while(!(TWCR & (1<<TWINT))){}
    //wait for radio data byte 1 to go out

    TWDR = RADIO_BYTE_4;
    //send pointer address to radio
    TWCR = TWCR_SEND;
    //poke TWINT to send address
    while(!(TWCR & (1<<TWINT))){}
    //wait for radio data byte 1 to go out

    TWDR = RADIO_BYTE_5;
    //send pointer address to radio
    TWCR = TWCR_SEND;
    //poke TWINT to send address
    while(!(TWCR & (1<<TWINT))){}
    //wait for radio data byte 1 to go out

    TWCR = TWCR_STOP;
    //finish transaction

    if(mute)
        PORTD |= 0x04; 
    else
        PORTD &= 0xFB;

    return(0);
    //return success value
}

uint8_t read_radio()
{
    uint8_t garbage;

    TWCR = TWCR_START;
    while(!(TWCR & (1<<TWINT))){} //wait for start condition to transmit
    
    TWDR = RADIO_READ; //send device addr, write bit set
    TWCR = (1<<TWINT)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT))){}

    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while(!(TWCR & (1<<TWINT))){}
    garbage = TWDR;

    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while(!(TWCR & (1<<TWINT))){}
    garbage = TWDR;

    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while(!(TWCR & (1<<TWINT))){}
    garbage = TWDR;
    
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while(!(TWCR & (1<<TWINT))){}
    uint8_t temp = TWDR;

    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while(!(TWCR & (1<<TWINT))){}
    garbage = TWDR;
    
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);//finish transaction
    return (temp >> 4);
}
