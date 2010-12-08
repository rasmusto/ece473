#include <avr/io.h>
#include <inttypes.h>

//function prototypes
void LED_write(uint8_t * segment_data);
uint8_t LED_convert(uint8_t digit);
