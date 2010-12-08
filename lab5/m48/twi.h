#include <util/twi.h>

//start condition transmitted
#define TW START            0x08
//repeated start condition transmitted
#define TW_REP_START        0x10
//SLA+W transmitted, ACK received
#define TW_MT_SLA_ACK       0x18    
//SLW_W transmitted, NACK received
#define TW_MT_SLA_NACK      0x20

//LM73 Addresses on the I2C bus
//Using LM73-0, address pin floating (datasheet pg. 9)
#define LM73_ADDRESS 0x90 
#define LM73_WRITE (LM73_ADDRESS | TW_WRITE) //LSB is a zero to write
#define LM73_READ  (LM73_ADDRESS | TW_READ)  //LSB is a one to read

//define the codes for actions to occur
#define TWCR_START  0xA4 //send start condition  
#define TWCR_STOP   0x94 //send stop condition   
#define TWCR_RACK   0xC4 //receive byte and return ack to slave  
#define TWCR_RNACK  0x84 //receive byte and return nack to slave
#define TWCR_SEND   0x84 //pokes the TWINT flag in TWCR and TWEN

#define LM73_PTR_TEMP         0x00  //LM73 temperature address
#define LM73_PTR_CONFIG       0x01  //LM73 configuration address
#define LM73_PTR_CTRL_STATUS  0x04  //LM73 ctrl and stat register

uint16_t rd_temp();
