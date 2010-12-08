#include <avr/io.h>
#include <inttypes.h>

#define COLON_ON_bm     0b11111100
#define COLON_OFF_bm    0b11111111
//function prototypes
void LED_write(uint8_t * segment_data);
uint8_t LED_convert(uint8_t digit);
