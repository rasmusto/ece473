// lab5_m128.c
// Torben Rasmussen
// 11.02.10
//
#define BAUD 9600
#define MYUBRR 103

#include "lab6_m128.h"
#include "/usr/lib/avr/include/avr/iom128.h" //for tons of tab completion

void segsum(uint16_t sum);
uint8_t chk_buttons(uint8_t button);
void setBar(uint8_t value);
uint16_t sec2mil(int32_t seconds);

//holds data to be sent to the segments. logic zero turns segment on
static uint8_t segment_data[5];
static char lcd_line1[16];
static char lcd_line2[16] = "Normal Mode."; 
uint16_t sum;
uint16_t count = 0;  //loop counter for updating each individual 7-seg
uint8_t mode = 1; //24-hr clock
uint16_t isr_count = 0;
static int32_t time = 3590; //seconds
static int32_t alarm = 3600; //seconds

static uint8_t signal_strength;

static uint8_t set_time_flag = 0;
static uint8_t set_alarm_flag = 0;
static uint8_t set_volume_flag = 1;
static uint8_t set_radio_flag = 1;
static uint16_t radio_freq = 887;

static uint8_t display_radio_count = 0;

static uint8_t time_mode = 0;
static uint8_t bar = 0;
static uint8_t volume = 1;
static uint8_t colon_blink_flag = 1;
static uint8_t lcd_line1_write_flag = 0;
static uint8_t lcd_line2_write_flag = 0;
static uint8_t music_playing_flag = 0;
volatile int ms = 0;
volatile uint16_t temperature;
volatile uint16_t temperature_old;
volatile uint16_t r_temperature;
volatile uint16_t r_temperature_temp;
volatile uint16_t r_temperature_old;

volatile uint8_t uart_data = 0;

static uint8_t display_adc_flag = 0;
static uint8_t display_radio_flag = 0;
static uint8_t display_bars_flag = 0;

static uint8_t alarm_armed_flag = 0;

static uint8_t alarm_playing_flag = 0;
static uint8_t snooze_timer = 0;

volatile uint8_t high_byte_flag = 1;

void USART_init(unsigned int ubrr);

static uint32_t adc_output;

static uint8_t spi_open = 1;

static uint32_t temp;

void tcnt0_init(void)
{
    ASSR   |=  (1<<AS0);    //ext osc TOSC
    TIMSK  |=  (1<<TOIE0);  //enable timer/counter0 overflow interrupt
    TCCR0  |=  (0<<WGM01) | (0<<WGM00) | (0<<COM00) | (0<<COM01) | (0<<CS02) | (0<<CS01) | (1<<CS00);//normal mode, no prescale
}

void tcnt2_init(void)
{
    TCCR2 |= (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (1 << CS20);
    //TIMSK |= (1 << OCIE2);
    //OCR2 = 0xFF;
    OCR2 = 0x00;
}

void tcnt3_init(void)
{
    TCCR3A |= (1<<WGM30) | (1<<COM3A1);
    TCCR3B |= (1<WGM32) | (1<<CS30);
    OCR3A = 0x3A;
}

void spi_init(void)
{
    SPCR  |=   (1<<SPE) | (1<<MSTR);    //set up SPI mode
    SPSR  |=   (1<<SPI2X);              // double speed operation
}//spi_init

void adc_init(void)
{
    /*
    //single shot mode
    ADCSRA |= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    //left adjust, use AVCC as reference
    ADMUX |= (1<<ADLAR) | (1<<REFS0);
    */

    //free running mode
    ADMUX |= (1<<REFS0);
    ADCSRA |= (1<<ADEN) | (1<<ADFR) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    ADCSRA |= (1<<ADSC);
}

void USART_init(unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    //enable tx and rx, enable rx complete interrupt
    UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
    //8 bits per character
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

void segsum(uint16_t sum)
{
    uint8_t ones;
    uint8_t tens;
    uint8_t hundreds;
    uint8_t thousands;
    //break up decimal sum into 4 digit-segments
    //these variables hold the decimal representation of each digit
    thousands = (sum / 1000) % 10;
    hundreds = (sum / 100) % 10;
    tens = (sum / 10) % 10;
    ones = (sum / 1) % 10;
    //blank out leading zero digits 
    //now move data to right place for misplaced colon position
    //this code also blanks out any leading zeros
    if(time_mode == 1 && thousands == 0 && hundreds == 0)
    {
        thousands = 1;
        hundreds = 2;
    }

    if(colon_blink_flag)
    {
        segment_data[4] = thousands;
        segment_data[3] = hundreds;
        segment_data[2] = COLON_ON_bm;
        segment_data[1] = tens;
        segment_data[0] = ones;
    }
    else
    {
        segment_data[4] = thousands;
        segment_data[3] = hundreds;
        segment_data[2] = COLON_OFF_bm;
        segment_data[1] = tens;
        segment_data[0] = ones;
    }

}//segment_sum

//******************************************************************************
//                            chk_buttons                                      
//Checks the state of the button number passed to it. It shifts in ones till   
//the button is pushed. Function returns a 1 only once per debounced button    
//push so a debounce and toggle function can be implemented at the same time.  
//Adapted to check all buttons from Ganssel's "Guide to Debouncing"            
//Expects active low pushbuttons on PINA port.  Debounce time is determined by 
//external loop delay times 12. 
//
//This code has been modified to work with a button passed in as a parameter (0-7)
uint8_t chk_buttons(uint8_t button) 
{
    static uint16_t state[8] = {0,0,0,0,0,0,0,0};
    state[button] = (state[button] << 1) | (! bit_is_clear(PINA, button)) | 0xE000;
    if (state[button] == 0xF000) return 1;
    return 0;
}
//******************************************************************************

//send 'value' to the bargraph display
void setBar(uint8_t value)
{
    if(spi_open)
    {
        spi_open = 0;
        //enable bar graph
        PORTB |= 0b01100000;
        PORTB &= 0b11101111;
        SPDR = value; 
        while(bit_is_clear(SPSR, SPIF)){}
        //disable bar graph
        PORTB &= 0b10001111;
        spi_open = 1;
    }
}

uint16_t sec2mil(int32_t seconds)
{
    uint16_t minutes = seconds / 60;
    uint16_t hours = minutes / 60;
    return ((minutes % 60) + 100 * (hours % 24));
}

uint16_t sec2hr(int32_t seconds)
{
    uint16_t minutes = seconds / 60;
    uint16_t hours = minutes / 60;
    return ((minutes % 60) + 100 * (hours % 12));
}

//USART0 rx complete
ISR(USART0_RX_vect)
{
    if(high_byte_flag)
    {
        uart_data = UDR0;
        r_temperature_temp = uart_data << 8;
        high_byte_flag = 0;
    }
    else
    {
        uart_data = UDR0;
        r_temperature_temp |= uart_data;
        high_byte_flag = 1;
        r_temperature = r_temperature_temp;
    }
}

ISR(TIMER0_OVF_vect) 
{
    static uint8_t knob1_state;
    static uint8_t knob1_old;
    static uint8_t knob2_state;
    static uint8_t knob2_old;
    static uint8_t data;

    uint8_t left_turn[4] = {3, 1, 0, 2};
    uint8_t right_turn[4] = {3, 2, 0, 1};

    static uint8_t knob1_hit_count = 4;
    static uint8_t knob2_hit_count = 4;

    /*
    adc_output = ADCW;
    OCR2 = 0xEF + adc_output;
    */


    temp = ADCW;

    if(temp < 40)
        temp = 40;
    if(temp > 850)
        temp = 850;

    //adc_output = temp - 275;
    adc_output = temp - 275;
    adc_output = 80 + (adc_output) / 3;
    adc_output -= 50;
    if (adc_output > 225)
        adc_output = 225;
    OCR2 = adc_output;

    ms++;
    if(ms % 4 == 0) { //used to be 8
        //for note duration (64th notes) 
        beat++;
    } 
    
    if(isr_count == 64){
        colon_blink_flag = 0;
        if (display_radio_count < 2)
            display_radio_count += 1;
        if (display_radio_count == 2)
            display_radio_flag = 0;
        //PORTF = 0x00;
    }

    if (isr_count == 128){
        temperature = rd_temp();
        if(temperature != temperature_old)
        {
            lcd_line1_write_flag = 1;
            temperature_old = temperature;
        }
        if(r_temperature != r_temperature_old)
        {
            lcd_line1_write_flag = 1;
            r_temperature_old = r_temperature;
        }
        colon_blink_flag = 1;
        isr_count = 0;
        //PORTF = 0x01;
        time += 1;

        if (snooze_timer > 0)
        {
            music_off();
            snooze_timer--;
            if(snooze_timer == 0 && alarm_armed_flag)
            {
                music_on();
                alarm_playing_flag = 1;
            }
        }

    }
    isr_count++;

    //stop colon blinking if setting time/alarm
    if(set_alarm_flag || set_time_flag){
        colon_blink_flag = 1;
    }

    //make PORTA an input port with pullups 
    DDRA = 0x00; //PORTA is an input
    PORTA = 0xFF; //Enable pullups
    //enable tristate buffer for pushbutton switches
    PORTB |= 0b01010000;
    PORTB &= 0b11011111;

    //now check each button and increment the count as needed
    _delay_us(10);   //this delay was necessary for the first button to be read

    //toggle set alarm mode
    if (chk_buttons(0) && !set_time_flag && !display_adc_flag && !display_radio_flag)
    {
        if(set_alarm_flag == 0){
            set_alarm_flag = 1;
            set_volume_flag = 0;
            bar |= 0b10000000;
            memcpy(lcd_line2, "Set alarm time...", 16);
            lcd_line2_write_flag = 1;
        }
        else
        {
            set_alarm_flag = 0;
            set_volume_flag = 1;
            bar &= 0b01111111;
            memcpy(lcd_line2, "Normal mode.", 16);
            lcd_line2_write_flag = 1;
        }
    }

    //toggle set clock mode
    if (chk_buttons(1) && !set_alarm_flag && !display_adc_flag && !display_radio_flag)
    { 
        if(set_time_flag == 0){
            set_time_flag = 1;
            set_volume_flag = 0;
            bar |= 0b01000000;
            memcpy(lcd_line2, "Set clock time...", 16);
            lcd_line2_write_flag = 1;
        }
        else
        {
            set_time_flag = 0;
            set_volume_flag = 1;
            bar &= 0b10111111;
            memcpy(lcd_line2, "Normal mode.", 16);
            lcd_line2_write_flag = 1;
        }
    }

    //play or stop music
    if (chk_buttons(2))
    {
        if(music_playing_flag)
        {
            music_off();
            memcpy(lcd_line2, "Stopping Music.", 16);
            music_playing_flag = 0;
        }
        else
        {
            music_on();
            memcpy(lcd_line2, "Playing Music.", 16);
            music_playing_flag = 1;
        }
    }

    //arm alarm
    if (chk_buttons(3))
    {
        if(alarm_armed_flag == 0)
        {
            alarm_armed_flag = 1;
            memcpy(lcd_line2, "Alarm is ON.", 16);
            lcd_line2_write_flag = 1;
        }
        else
        {
            alarm_armed_flag = 0;
            memcpy(lcd_line2, "Alarm is OFF.", 16);
            lcd_line2_write_flag = 1;
            music_off();
        }

    }

    //snooze if alarm is playing
    if (chk_buttons(4))
    {
        if(alarm_playing_flag && alarm_armed_flag)
        {
            snooze_timer = 10;
            memcpy(lcd_line2, "Snooze is on.", 16);
            lcd_line2_write_flag = 1;
        }
    }
    
    if (chk_buttons(5) && !set_time_flag && !set_alarm_flag && !display_adc_flag)
    {
        if(display_bars_flag == 0)
        {
            display_bars_flag = 1;
            memcpy(lcd_line2, "Showing radio station", 16);
            lcd_line2_write_flag = 1;
        }
        else
        {
            display_bars_flag = 0;
            memcpy(lcd_line2, "Normal mode.", 16);
            lcd_line2_write_flag = 1;
        }
    }

    //output adc to leds
    if (chk_buttons(6) && !set_time_flag && !set_alarm_flag && !display_radio_flag)
    {
        if(display_adc_flag == 0){
            display_adc_flag = 1;
            memcpy(lcd_line2, "Showing adc output", 16);
            lcd_line2_write_flag = 1;
        }
        else
        {
            display_adc_flag = 0;
            memcpy(lcd_line2, "Normal mode.", 16);
            lcd_line2_write_flag = 1;
        }
    }

    //toggle 24/12 hour modes
    if (chk_buttons(7))
    {
        if(time_mode == 0)
        {
            time_mode = 1;
            bar |= 0b00000001;
        }
        else
        {
            time_mode = 0;
            bar &= 0b11111110;
        }
    }

    //disable tristate buffer
    PORTB &= 0b10001111;

    //Wiggle PB0
    PORTB &= 0b11111110;
    PORTB |= 0b00000001;

    SPDR = 0xFF;
    data = SPDR;
    SPDR = 0xFF;

    knob1_state = data & 0b00000011;
    knob2_state = data & 0b00001100;
    knob2_state = knob2_state >> 2;

    //If either knob has changed state, check which direction
    //it went
    uint8_t i = 0;
    
    if (knob1_state != knob1_old){
        while (left_turn[i%4] != knob1_state){ i++; }
        if (knob1_old == left_turn[(i-1)%4]){
            knob1_hit_count--;
        }
    
        while (right_turn[i%4] != knob1_state){ i++; }
        if (knob1_old == right_turn[(i-1)%4]){
            knob1_hit_count++;
        }
    }
    
    if (knob2_state != knob2_old){
        while (left_turn[i%4] != knob2_state){ i++; }
        if (knob2_old == left_turn[(i-1)%4]){
            knob2_hit_count--;
        }
    
        while (right_turn[i%4] != knob2_state){ i++; }
        if (knob2_old == right_turn[(i-1)%4]){
            knob2_hit_count++;
        }
    }
 
    if(knob1_hit_count == 7){
        knob1_hit_count = 4;
        if(set_time_flag)
            time += 60;
        if(set_alarm_flag)
            alarm += 60;
        if(set_volume_flag)
        {
            volume += 1;
            if(volume == 9)
                volume = 8;
        }
    }

    if(knob1_hit_count == 1)
    {
        knob1_hit_count = 4;
        if(set_time_flag)
            time -= 60;
        if(set_alarm_flag)
            alarm -= 60;
        if(set_volume_flag)
        {
            volume -= 1;
            if(volume == 0)
                volume = 1;
        }
    }
    
    if(knob2_hit_count == 7){
        knob2_hit_count = 4;
        if(set_time_flag)
            time += 3600;
        if(set_alarm_flag)
            alarm += 3600;
        if(set_radio_flag)
        {
            radio_freq += 2;
            if (radio_freq == 1081)
                radio_freq = 871;
            send_radio(radio_freq);
            display_radio_count = 0;
            display_radio_flag = 1;
            signal_strength = read_radio();
        }
    }

    if(knob2_hit_count == 1)
    {
        knob2_hit_count = 4;
        if(set_time_flag)
            time -= 3600;
        if(set_alarm_flag)
            alarm -= 3600;
        if(set_radio_flag)
        {
            radio_freq -= 2;
            if (radio_freq == 869)
                radio_freq = 1079;
            send_radio(radio_freq);
            display_radio_count = 0;
            display_radio_flag = 1;
            signal_strength = read_radio();
        }
    }

    knob1_old = knob1_state;
    knob2_old = knob2_state;

}

ISR(TIMER2_COMP_vect)
{
    /*
    if(temp > 60){
        adc_output = temp;
        OCR2 = 80 + adc_output;
    }
    */
}

int main(void)
{
    
    music_init();

    //port initialization
    DDRB = 0xFF;    //set port B as an output
    DDRD = 0xFF;    //set port D as an output
    DDRE = 0xFF;    //set port E as an output
    DDRF = 0x08;    //set port F as an output

    PORTF = 0x01;   //set pullup on PF0

    TWBR = 0x03;

    spi_init();    //initialize SPI port
    tcnt0_init();  //initialize counter timer 0
    tcnt2_init();  //initialize counter timer 2
    tcnt3_init();  //initialize counter timer 3
    adc_init();
    lcd_init();
    USART_init(MYUBRR);
    setBar(0b10101010);

    sei();         //enable interrupts before entering loop

    sum = 0;
    ms = 0;
    beat = 0;

    send_radio(887);
    PORTD &= unmute;

    while(1)
    {
        //uint16_t temperature = rd_temp();

        if (count > 4) { count = 0; }

        //normal time display
        if (!set_time_flag && !set_alarm_flag)
        {
            if(time_mode == 1)
                segsum(sec2hr(time));
            else
                segsum(sec2mil(time));
        }

        //time set mode
        if(set_time_flag)
        {
            if(time_mode == 1)
                segsum(sec2hr(time));
            else
                segsum(sec2mil(time));
        }

        //alarm set mode
        if(set_alarm_flag)
        {
            if(time_mode == 1)
                segsum(sec2hr(alarm));
            else
                segsum(sec2mil(alarm));
        }
        
        //adc display mode
        if(display_adc_flag)
        {
            segsum(adc_output);
            //segsum(temp);
        }

        if(display_radio_flag)
        {
            segsum(radio_freq);
        }

        if(display_bars_flag)
        {
            segsum(signal_strength);
        }

        if (alarm_armed_flag && time == alarm){
            music_on();
            alarm_playing_flag = 1;
        }

        if (time >= 86400){
            time = time - 86400;
        }

        if (time < 0){
            time = 86400 + time;
        }

        if (alarm >= 86400){
            alarm = alarm - 86400;
        }

        if (alarm < 0){
            alarm = 86400 + alarm;
        }

        if(ms % 100 == 0)
            setBar(1 << (volume - 1));

        uint8_t digit = LED_convert(segment_data[count]);

        //make PORTA an output
        DDRA = 0xFF;
        //turn off leds
        PORTA = 0xFF;
        //send PORTB the digit to display
        uint8_t led_sel = count << 4;
        PORTB |= led_sel;  //set PORTB4-6 to select digit
        PORTB &= led_sel & 0b01110000;
        //send 7 segment code to LED segments
        PORTA = digit; 

        //update lcd line one
        if(lcd_line1_write_flag){

            char lcd_str_h[16];
            char lcd_str_l[16];
            div_t fp_adc_result = div(temperature, 4); //do division by 205 (204.8 to be exact)
            itoa(fp_adc_result.quot, lcd_str_h, 10); //convert non-fractional part to ascii string
            div_t fp_low_result = div((fp_adc_result.rem*100), 4); //get fraction into non-fractional form
            itoa(fp_low_result.quot, lcd_str_l, 10); //convert fractional part to ascii string

            char lcd_str2_h[16];
            char lcd_str2_l[16];
            div_t fp_adc_result2 = div(r_temperature, 4); //do division by 205 (204.8 to be exact)
            itoa(fp_adc_result2.quot, lcd_str2_h, 10); //convert non-fractional part to ascii string
            div_t fp_low_result2 = div((fp_adc_result2.rem*100), 4); //get fraction into non-fractional form
            itoa(fp_low_result2.quot, lcd_str2_l, 10); //convert fractional part to ascii string

            clear_display();
            cursor_home();

            string2lcd(lcd_str_h); //write upper half
            char2lcd('.'); //write decimal point
            string2lcd(lcd_str_l); //write lower half

            char2lcd(0x20);

            string2lcd(lcd_str2_h); //write upper half
            char2lcd('.'); //write decimal point
            string2lcd(lcd_str2_l); //write lower half

            //char buffer[16];
            //itoa(uart_data, buffer, 10);
            //itoa(temperature, buffer, 10);
            //string2lcd(buffer);
            lcd_line1_write_flag = 0;

            home_line2();
            string2lcd(lcd_line2);
        }

        //update lcd line two
        /*
        if(lcd_line2_write_flag){
            home_line2();
            string2lcd(lcd_line2);
            lcd_line2_write_flag = 0;
        }
        */

        //update digit to display
        OCR3A = volume * 32 - 1;
        count++;
    }
}//main
