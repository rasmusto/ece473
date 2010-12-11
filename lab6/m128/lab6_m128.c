// lab5_m128.c
// Torben Rasmussen
// 11.02.10
//
#define BAUD 9600
#define MYUBRR 103

#include "lab6_m128.h"
#include "/usr/lib/avr/include/avr/iom128.h" //for tons of tab completion

void segsum(uint16_t sum);
void setBar(uint8_t value);
uint8_t chk_buttons(uint8_t button);
uint16_t sec2mil(int32_t seconds);
uint16_t sec2hr(int32_t seconds);

void USART_init(unsigned int ubrr);

//holds data to be sent to the segments. logic zero turns segment on
volatile uint8_t segment_data[5];
char lcd_line1[32];
char lcd_line2[32] = "Normal Mode";
volatile uint16_t sum;
volatile uint16_t count = 0;  //loop counter for updating each individual 7-seg
volatile uint8_t mode = 1; //24-hr clock
volatile uint16_t isr_count = 0;

//time variables
volatile int32_t time = 3590; //seconds
volatile int32_t alarm = 3600; //seconds
volatile int ms = 0;

//user interface flags
volatile uint8_t set_time_flag = 0;
volatile uint8_t set_alarm_flag = 0;
volatile uint8_t set_volume_flag = 1;
volatile uint8_t set_radio_flag = 1;
volatile uint8_t radio_on_flag = 0;
volatile uint8_t alarm_mode = 0;
volatile uint8_t time_mode = 0;
volatile uint8_t display_adc_flag = 0;
volatile uint8_t display_radio_flag = 0;
volatile uint8_t display_bars_flag = 0;
volatile uint8_t alarm_armed_flag = 0;
volatile uint8_t alarm_playing_flag = 0;
volatile uint8_t snooze_timer = 0;
volatile uint16_t snooze_duration = 10;

//radio variables
volatile int8_t volume = 5;
volatile uint16_t radio_freq = 887;
volatile uint8_t signal_strength;
volatile uint8_t display_radio_count = 0;

//led variables
volatile uint8_t colon_blink_flag = 1;

//lcd variables
volatile uint8_t lcd_line1_write_flag = 0;
volatile uint8_t lcd_line2_write_flag = 0;
volatile uint8_t lcd_count = 0;

//temperature variables
volatile uint16_t temperature;
volatile uint16_t temperature_old;
volatile uint16_t r_temperature;
volatile uint16_t r_temperature_temp;
volatile uint16_t r_temperature_old;

//light sensor variables
volatile uint32_t adc_temp;
volatile uint32_t adc_output;

//uart variables
volatile uint8_t uart_data = 0;
volatile uint8_t high_byte_flag = 1;

//spi variables
volatile uint8_t spi_open = 1;

void tcnt0_init(void)
{
    ASSR   |=  (1<<AS0);    //ext osc TOSC
    TIMSK  |=  (1<<TOIE0);  //enable timer/counter0 overflow interrupt
    TCCR0  |=  (0<<WGM01) | (0<<WGM00) | (0<<COM00) | (0<<COM01) | (0<<CS02) | (0<<CS01) | (1<<CS00);//normal mode, no prescale
}

void tcnt2_init(void)
{
    TCCR2 |= (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (1 << CS20);
    OCR2 = 0x00;
}

void tcnt3_init(void)
{
    TCCR3A |= (1<<WGM30) | (1<<COM3A1);
    TCCR3B |= (1<WGM32) | (1<<CS30);
}

void spi_init(void)
{
    SPCR  |=   (1<<SPE) | (1<<MSTR);    //set up SPI mode
    SPSR  |=   (1<<SPI2X);              // double speed operation
}//spi_init

void adc_init(void)
{
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

    thousands = (sum / 1000) % 10;
    hundreds = (sum / 100) % 10;
    tens = (sum / 10) % 10;
    ones = (sum / 1) % 10;

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

//check the status of a button
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

    adc_temp = ADCW;

    if(adc_temp < 40)
        adc_temp = 40;
    if(adc_temp > 850)
        adc_temp = 850;

    adc_output = adc_temp - 275;
    adc_output = 80 + (adc_output) / 3;
    adc_output -= 50;
    if (adc_output > 225)
        adc_output = 225;
    OCR2 = adc_output;

    ms++;
    if(ms % 4 == 0) {
        //for note duration (64th notes) 
        beat++;
    } 
    
    if(isr_count == 64){
        colon_blink_flag = 0;
        if (display_radio_count < 2)
            display_radio_count += 1;
        if (display_radio_count == 2)
            display_radio_flag = 0;
        signal_strength = read_radio();
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
        time += 1;

        if (snooze_timer > 0)
        {
            music_off();
            /*
            if(!radio_on_flag)
            { send_radio(radio_freq, 1); }
            */
            snooze_timer--;
            if(snooze_timer == 0 && alarm_armed_flag)
            {
                if(alarm_mode == 0)
                {
                    send_radio(radio_freq, 1);
                    music_on();
                    alarm_playing_flag = 1;
                }
                else
                {
                    send_radio(radio_freq, 0);
                    alarm_playing_flag = 1;
                }
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

    //read buttons states
    uint8_t button0_state = chk_buttons(0);
    uint8_t button1_state = chk_buttons(1);
    uint8_t button2_state = chk_buttons(2);
    uint8_t button3_state = chk_buttons(3);
    uint8_t button4_state = chk_buttons(4);
    uint8_t button5_state = chk_buttons(5);
    uint8_t button6_state = chk_buttons(6);
    uint8_t button7_state = chk_buttons(7);

    //disable tristate buffer
    PORTB &= 0b10001111;

    /*
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

    //update digit to display
    count++;
    */

    //toggle set alarm mode
    if (button0_state && !set_time_flag && !display_adc_flag && !display_radio_flag)
    {
        if(set_alarm_flag == 0){
            set_alarm_flag = 1;
            set_volume_flag = 0;
            set_radio_flag = 0;
            memcpy(lcd_line2, "Set Alarm Time", 16);
            lcd_line2_write_flag = 1;
        }
        else
        {
            set_alarm_flag = 0;
            set_volume_flag = 1;
            set_radio_flag = 1;
            memcpy(lcd_line2, "Normal Mode", 16);
            lcd_line2_write_flag = 1;
        }
    }

    //toggle set clock mode
    if (button1_state && !set_alarm_flag && !display_adc_flag && !display_radio_flag)
    { 
        if(set_time_flag == 0){
            set_time_flag = 1;
            set_volume_flag = 0;
            set_radio_flag = 0;
            memcpy(lcd_line2, "Set Clock Time", 16);
            lcd_line2_write_flag = 1;
        }
        else
        {
            set_time_flag = 0;
            set_volume_flag = 1;
            set_radio_flag = 1;
            memcpy(lcd_line2, "Normal mode", 16);
            lcd_line2_write_flag = 1;
        }
    }

    //change alarm mode
    if (button2_state)
    {
        if(alarm_mode == 0)
        {
            alarm_mode = 1;
            memcpy(lcd_line2, "Alarm - Radio", 16);
            lcd_line2_write_flag = 1;
        }
        else
        {
            alarm_mode = 0;
            memcpy(lcd_line2, "Alarm - Tone", 16);
            lcd_line2_write_flag = 1;
        }
    }

    //arm alarm
    if (button3_state && !alarm_playing_flag)
    {
        if(alarm_armed_flag)
        {
            alarm_playing_flag = 0;
            alarm_armed_flag = 0;
            memcpy(lcd_line2, "Alarm - OFF", 16);
            lcd_line2_write_flag = 1;
            if(alarm_mode == 0)
            {
                music_off();
            }
            else
            {
                send_radio(radio_freq, 1);
                radio_on_flag = 0;
            }
        }
        else
        {
            //alarm_playing_flag = 0;
            alarm_armed_flag = 1;
            memcpy(lcd_line2, "Alarm - ON", 16);
            lcd_line2_write_flag = 1;
        }

    }

    //snooze if alarm is playing
    if (button4_state)
    {
        if(alarm_playing_flag && alarm_armed_flag)
        {
            alarm_playing_flag = 0;
            snooze_timer = snooze_duration;
            memcpy(lcd_line2, "Snooze - On", 16);
            lcd_line2_write_flag = 1;
        }
        else
        {
            memcpy(lcd_line2, "Snooze - N/A", 16);
            lcd_line2_write_flag = 1;
        }
    }

    //toggle snooze duration
    if (button5_state)
    {
        if(snooze_duration == 10)
        {
            snooze_duration = 60;
            memcpy(lcd_line2, "Snooze - 1m", 16);
            lcd_line2_write_flag = 1;
        }
        else
        {
            snooze_duration = 10;
            memcpy(lcd_line2, "Snooze - 10s", 16);
            lcd_line2_write_flag = 1;
        }
    }
    
    //enable radio
    if (button6_state && !alarm_playing_flag)
    {
        if(radio_on_flag == 0)
        {
            radio_on_flag = 1;
            memcpy(lcd_line2, "Radio - ON", 16);
            lcd_line2_write_flag = 1;
            send_radio(radio_freq, 0);
            //signal_strength = read_radio();
        }
        else
        {
            radio_on_flag = 0;
            memcpy(lcd_line2, "Radio - OFF", 16);
            lcd_line2_write_flag = 1;
            send_radio(radio_freq, 1);
            //signal_strength = read_radio();
        }
    }

    //toggle 24/12 hour modes
    if (button7_state)
    {
        if(time_mode == 0)
        {
            time_mode = 1;
            memcpy(lcd_line2, "12 Hour Mode", 16);
            lcd_line2_write_flag = 1;
        }
        else
        {
            time_mode = 0;
            memcpy(lcd_line2, "24 Hour Mode", 16);
            lcd_line2_write_flag = 1;
        }
    }

    //Wiggle PB0
    PORTB &= 0b11111110;
    PORTB |= 0b00000001;

    SPDR = data;
    data = SPDR;
    setBar(data);

    knob1_state = data & 0b00000011;
    knob2_state = data & 0b00001100;
    knob2_state = knob2_state >> 2;

    if (knob1_state == 0b01 && knob1_old == 0b00)
    {
        if(set_time_flag) time += 3600;
        if(set_alarm_flag) alarm += 3600;
        if(set_volume_flag)
        {
            volume++;
            if(volume == 16)
                volume = 15;
        }
    }

    if (knob1_state == 0b11 && knob1_old == 0b10)
    {
        if(set_time_flag) time -= 3600;
        if(set_alarm_flag) alarm -= 3600;
        if(set_volume_flag)
        {
            volume--;
            if(volume == -1)
                volume = 0;
        }
    }

    if (knob2_state == 0b01 && knob2_old == 0b00)
    {
        if(set_time_flag) time += 60;
        if(set_alarm_flag) alarm += 60;
        if(set_radio_flag)
        {
            radio_freq += 2;
            if (radio_freq == 1081)
                radio_freq = 871;
            display_radio_count = 0;
            display_radio_flag = 1;
            send_radio(radio_freq, !radio_on_flag);
            //signal_strength = read_radio();
        }
    }

    if (knob2_state == 0b11 && knob2_old == 0b10)
    {
        if(set_time_flag) time -= 60;
        if(set_alarm_flag) alarm -= 60;
        if(set_radio_flag)
        {
            radio_freq -= 2;
            if (radio_freq == 869)
                radio_freq = 1079;
            display_radio_count = 0;
            display_radio_flag = 1;
            send_radio(radio_freq, !radio_on_flag);
            //signal_strength = read_radio();
        }
    }
    knob1_old = knob1_state;
    knob2_old = knob2_state;
}

int main(void)
{
    char zeros[8];
    int i;
    for(i = 0; i < 8; i++)
        zeros[i] = 0x20;

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

    sei();         //enable interrupts before entering loop

    sum = 0;
    ms = 0;
    beat = 0;

    while(1)
    {
        if (count > 4) { count = 0; }

        //normal time display
        if (!set_time_flag && !set_alarm_flag)
        {
            if(time_mode == 1) segsum(sec2hr(time));
            else segsum(sec2mil(time));
        }

        //time set mode
        if(set_time_flag)
        {
            if(time_mode == 1) segsum(sec2hr(time));
            else segsum(sec2mil(time));
        }

        //alarm set mode
        if(set_alarm_flag)
        {
            if(time_mode == 1) segsum(sec2hr(alarm));
            else segsum(sec2mil(alarm));
        }

        //adc display mode
        if(display_adc_flag) { segsum(adc_output); }

        //display radio station
        if(display_radio_flag) { segsum(radio_freq); }

        //display signal strength
        if(display_bars_flag) { segsum(signal_strength); }

        //sound alarm
        if (alarm_armed_flag && time == alarm){
            if(alarm_mode == 0)
            {
                send_radio(radio_freq, 1);
                music_on();
                alarm_playing_flag = 1;
            }
            else
            {
                send_radio(radio_freq, 0);
                alarm_playing_flag = 1;
                radio_on_flag = 1;
            }
        }

        //bound time and alarm
        if (time >= 86400){ time -= 86400; }
        if (time < 0){ time += 86400; }

        if (alarm >= 86400){ alarm -= 86400; } 
        if (alarm < 0){ alarm += 86400; }

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
        if(lcd_line1_write_flag | lcd_line2_write_flag)
        {
            char lcd_str_h[16];
            char lcd_str_l[16];
            div_t fp_adc_result = div(temperature, 4); //do division by 205 (204.8 to be exact)
            itoa(fp_adc_result.quot, lcd_str_h, 10); //convert non-fractional part to ascii string
            div_t fp_low_result = div((fp_adc_result.rem*100), 4); //get fraction into non-fractional form
            if(fp_low_result.quot == 0)
                memcpy(lcd_str_l,  "00", 3);
            else
                itoa(fp_low_result.quot, lcd_str_l, 10); //convert fractional part to ascii string

            char lcd_str2_h[16];
            char lcd_str2_l[16];
            div_t fp_adc_result2 = div(r_temperature, 4); //do division by 205 (204.8 to be exact)
            itoa(fp_adc_result2.quot, lcd_str2_h, 10); //convert non-fractional part to ascii string
            div_t fp_low_result2 = div((fp_adc_result2.rem*100), 4); //get fraction into non-fractional form
            if(fp_low_result2.quot == 0)
                memcpy(lcd_str2_l, "00", 3);
            else
                itoa(fp_low_result2.quot, lcd_str2_l, 10); //convert fractional part to ascii string

            cursor_home();

            sprintf(lcd_line1, "%s.%s %s.%s", lcd_str_h, lcd_str_l, lcd_str2_h, lcd_str2_l);
            uint8_t num_bars;
            num_bars = signal_strength / 3;
            switch(num_bars)
            {
                case 5:
                    sprintf(lcd_line1, "%s*****", lcd_line1);
                    break;
                case 4:
                    sprintf(lcd_line1, "%s ****", lcd_line1);
                    break;
                case 3:
                    sprintf(lcd_line1, "%s  ***", lcd_line1);
                    break;
                case 2:
                    sprintf(lcd_line1, "%s   **", lcd_line1);
                    break;
                case 1:
                    sprintf(lcd_line1, "%s    *", lcd_line1);
                    break;
                default:
                    break;
            }

            strcat(lcd_line1, zeros);
            lcd_line1[16] = '\0';

            string2lcd(lcd_line1);
            lcd_line1_write_flag = 0;

            strcat(lcd_line2, zeros);
            lcd_line2[16] = '\0';

            home_line2();
            string2lcd(lcd_line2);
            lcd_line2_write_flag = 0;
        }

        //set pwm duty cycle for volume
        OCR3A = (volume * 16);

        //update digit to display
        count++;
    }
}//main
