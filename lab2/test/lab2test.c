#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void segsum(uint16_t sum);

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
    if(10 < sum && sum < 100){
        numDigits = 2;
    }
    if(100 < sum && sum < 1000){
        numDigits = 3;
    }
    if(1000 < sum && sum < 10000){
        numDigits = 4;
    }
    //break up decimal sum into 4 digit-segments
    thousands = (sum / 1000) % 10;
    hundreds = (sum / 100) % 10;
    tens = (sum / 10) % 10;
    ones = (sum / 1) % 10;
    printf("thousands = %d\n", thousands); 
    printf("hundreds = %d\n", hundreds); 
    printf("tens = %d\n", tens); 
    printf("ones = %d\n", ones); 
    //blank out leading zero digits 
    //now move data to right place for misplaced colon position
}//segment_sum
int main(int argc, const char *argv[])
{
    segsum(13802);
    return 0;
}
