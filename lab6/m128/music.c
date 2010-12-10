#include "lab6_m128.h"

void song0(uint16_t note) { //(Memories of green)
    switch (note) {
        //pickup
        case  0: play_note('B', 0, 3, 8);
                 break;
        //measure 1
        case  1: play_note('E', 0, 4, 12);
                 break;
        case  2: play_note('G', 1, 4, 12);
                 break;
        case  3: play_note('G', 0, 4, 16);
                 break;
        case  4: play_note('B', 0, 3, 8);
                 break;
        case  5: play_note('G', 1, 4, 8);
                 break;
        //tie
        case  6: play_note('D', 0, 4, 48);
                 break;
        case  7: play_note('A', 0, 3, 12);
                 break;
        case  8: play_note('B', 0, 3, 4);
                 break;
        case  9: play_note('E', 0, 4, 4);
                 break;
        case 10: play_note('A', 0, 4, 4);
                 break;
        case 11: play_note('B', 0, 4, 10);
                 break;
        case 12: play_rest(2);
                 break;
        case 13: play_note('B', 0, 4, 12);
                 break;
        case 14: play_note('A', 0, 4, 4);
                 break;
        case 15: play_note('A', 1, 4, 4);
                 break;
        case 16: play_note('E', 0, 4, 80);
                 break;
        //measure 2
        default: notes=-1;
    }
}

void song1(uint16_t note) { //600AD
    switch (note) {
        //pickup
        case  0: play_note('D', 0, 3, 16);
                 break;
        case  1: play_note('E', 0, 3, 16);
                 break;
                 //measure 1
        case  2: play_note('F', 0, 3, 8);
                 break;
        case  3: play_note('E', 0, 3, 8);
                 break;
        case  4: play_note('C', 0, 3, 8);
                 break;
        case  5: play_note('A', 0, 2, 8);
                 break;
        case  6: play_note('D', 0, 3, 16);
                 break;
        case  7: play_rest(8);
                 break;
        case  8: play_note('D', 0, 3, 8);
                 break;
                 //measure 2
        case  9: play_note('C', 0, 3, 8);
                 break;
        case 10: play_note('A', 0, 2, 8);
                 break;
        case 11: play_note('F', 0, 2, 8);
                 break;
        case 12: play_note('G', 0, 2, 8);
                 break;
        case 13: play_note('A', 0, 2, 16);
                 break;
        case 14: play_note('G', 0, 2, 16);
                 break;
                 //measure 3
        case 15: play_note('E', 0, 2, 8);
                 break;
        case 16: play_note('F', 0, 2, 8);
                 break;
        case 17: play_note('A', 0, 2, 8);
                 break;
        case 18: play_note('C', 0, 3, 8);
                 break;
        case 19: play_note('E', 0, 3, 16);
                 break;
        case 20: play_note('F', 0, 3, 16);
                 break;
                 //measure 4
        case 21: play_note('C', 0, 3, 48);
                 break;
        case 22: play_note('E', 0, 3, 16);
                 break;
                 //measure 5
        case 23: play_note('F', 0, 3, 8);
                 break;
        case 24: play_note('E', 0, 3, 8);
                 break;
        case 25: play_note('C', 0, 3, 8);
                 break;
        case 26: play_note('A', 0, 2, 8);
                 break;
        case 27: play_note('D', 0, 3, 16);
                 break;
        case 28: play_rest(8);
                 break;
        case 29: play_note('D', 0, 3, 8);
                 break;
                 //measure 6
        case 30: play_note('C', 0, 3, 8);
                 break;
        case 31: play_note('A', 0, 2, 8);
                 break;
        case 32: play_note('F', 0, 2, 8);
                 break;
        case 33: play_note('G', 0, 2, 8);
                 break;
        case 34: play_note('A', 0, 2, 16);
                 break;
        case 35: play_note('G', 0, 2, 16);
                 break;
        case 36: play_note('E', 0, 2, 8);
                 break;
        case 37: play_note('F', 0, 2, 8);
                 break;
        case 38: play_note('A', 0, 2, 8);
                 break;
        case 39: play_note('C', 0, 3, 8);
                 break;
        case 40: play_note('E', 0, 3, 8);
                 break;
        case 41: play_note('D', 0, 3, 8);
                 break;
        case 42: play_note('C', 0, 3, 8);
                 break;
        case 43: play_note('D', 0, 3, 6);
                 break;
        case 44: play_rest(2);
                 break;
        case 45: play_note('D', 0, 3, 30);
                 break;
        case 46: play_rest(2);
                 break;
        default: notes=-1;
    }
}

        
        

void play_song(uint8_t song, uint8_t note) {
    switch (song) {
        case 0: song0(note); //memories of green
                break;
        case 1: song1(note); //600AD
                break;
        default: song0(note);
    }
}

void play_rest(uint8_t duration) {
    //mute for duration
    //duration is in 64th notes at 120bpm
    PORTD |= mute;
    beat=0;
    max_beat = duration;
}

void play_note(char note, uint8_t flat, uint8_t octave, uint8_t duration) {
    //pass in the note, it's key, the octave they want, and a duration
    //function sets the value of OCR1A and the timer
    //note must be A-G
    //flat must be 1 (for flat) or 0 (for natural) (N/A on C or F)
    //octave must be 0-8 (0 is the lowest, 8 doesn't sound very good)
    //duration is in 64th notes at 120bpm
    //e.g. play_note('D', 1, 0, 16)
    //this would play a Db, octave 0 for 1 quarter note
    //120 bpm (every 32ms inc beat)
    PORTD &= unmute;      //unmute (just in case)
    beat = 0;             //reset the beat counter
    max_beat = duration;  //set the max beat
    switch (octave) {
        case 0: switch (note) {
                    case 'A': if(flat){OCR1A=Ab0;}
                                  else {OCR1A=A0;}
                                  break;
                    case 'B': if(flat){OCR1A=Bb0;}
                                  else {OCR1A=B0;}
                                  break;
                    case 'C': OCR1A=C0;
                              break;
                    case 'D': if(flat){OCR1A=Db0;}
                                  else {OCR1A=D0;}
                                  break;
                    case 'E': if(flat){OCR1A=Eb0;}
                                  else {OCR1A=E0;}
                                  break;
                    case 'F': OCR1A=F0;
                              break;
                    case 'G': if(flat){OCR1A=Gb0;}
                                  else {OCR1A=G0;}
                                  break;
                } 
                break;
        case 1: switch (note) {
                    case 'A': if(flat){OCR1A=Ab1;}
                                  else {OCR1A=A1;}
                                  break;
                    case 'B': if(flat){OCR1A=Bb1;}
                                  else {OCR1A=B1;}
                                  break;
                    case 'C': OCR1A=C1;
                              break;
                    case 'D': if(flat){OCR1A=Db1;}
                                  else {OCR1A=D1;}
                                  break;
                    case 'E': if(flat){OCR1A=Eb1;}
                                  else {OCR1A=E1;}
                                  break;
                    case 'F': OCR1A=F1;
                              break;
                    case 'G': if(flat){OCR1A=Gb1;}
                                  else {OCR1A=G1;}
                                  break;
                } 
                break;
        case 2: switch (note) {
                    case 'A': if(flat){OCR1A=Ab2;}
                                  else {OCR1A=A2;}
                                  break;
                    case 'B': if(flat){OCR1A=Bb2;}
                                  else {OCR1A=B2;}
                                  break;
                    case 'C': OCR1A=C2;
                              break;
                    case 'D': if(flat){OCR1A=Db2;}
                                  else {OCR1A=D2;}
                                  break;
                    case 'E': if(flat){OCR1A=Eb2;}
                                  else {OCR1A=E2;}
                                  break;
                    case 'F': OCR1A=F2;
                              break;
                    case 'G': if(flat){OCR1A=Gb2;}
                                  else {OCR1A=G2;}
                                  break;
                } 
                break;
        case 3: switch (note) {
                    case 'A': if(flat){OCR1A=Ab3;}
                                  else {OCR1A=A3;}
                                  break;
                    case 'B': if(flat){OCR1A=Bb3;}
                                  else {OCR1A=B3;}
                                  break;
                    case 'C': OCR1A=C3;
                              break;
                    case 'D': if(flat){OCR1A=Db3;}
                                  else {OCR1A=D3;}
                                  break;
                    case 'E': if(flat){OCR1A=Eb3;}
                                  else {OCR1A=E3;}
                                  break;
                    case 'F': OCR1A=F3;
                              break;
                    case 'G': if(flat){OCR1A=Gb3;}
                                  else {OCR1A=G3;}
                                  break;
                } 
                break;
        case 4: switch (note) {
                    case 'A': if(flat){OCR1A=Ab4;}
                                  else {OCR1A=A4;}
                                  break;
                    case 'B': if(flat){OCR1A=Bb4;}
                                  else {OCR1A=B4;}
                                  break;
                    case 'C': OCR1A=C4;
                              break;
                    case 'D': if(flat){OCR1A=Db4;}
                                  else {OCR1A=D4;}
                                  break;
                    case 'E': if(flat){OCR1A=Eb4;}
                                  else {OCR1A=E4;}
                                  break;
                    case 'F': OCR1A=F4;
                              break;
                    case 'G': if(flat){OCR1A=Gb4;}
                                  else {OCR1A=G4;}
                                  break;
                } 
                break;
        case 5: switch (note) {
                    case 'A': if(flat){OCR1A=Ab5;}
                                  else {OCR1A=A5;}
                                  break;
                    case 'B': if(flat){OCR1A=Bb5;}
                                  else {OCR1A=B5;}
                                  break;
                    case 'C': OCR1A=C5;
                              break;
                    case 'D': if(flat){OCR1A=Db5;}
                                  else {OCR1A=D5;}
                                  break;
                    case 'E': if(flat){OCR1A=Eb5;}
                                  else {OCR1A=E5;}
                                  break;
                    case 'F': OCR1A=F5;
                              break;
                    case 'G': if(flat){OCR1A=Gb5;}
                                  else {OCR1A=G5;}
                                  break;
                } 
                break;
        case 6: switch (note) {
                    case 'A': if(flat){OCR1A=Ab6;}
                                  else {OCR1A=A6;}
                                  break;
                    case 'B': if(flat){OCR1A=Bb6;}
                                  else {OCR1A=B6;}
                                  break;
                    case 'C': OCR1A=C6;
                              break;
                    case 'D': if(flat){OCR1A=Db6;}
                                  else {OCR1A=D6;}
                                  break;
                    case 'E': if(flat){OCR1A=Eb6;}
                                  else {OCR1A=E6;}
                                  break;
                    case 'F': OCR1A=F6;
                              break;
                    case 'G': if(flat){OCR1A=Gb6;}
                                  else {OCR1A=G6;}
                                  break;
                } 
                break;
        case 7: switch (note) {
                    case 'A': if(flat){OCR1A=Ab7;}
                                  else {OCR1A=A7;}
                                  break;
                    case 'B': if(flat){OCR1A=Bb7;}
                                  else {OCR1A=B7;}
                                  break;
                    case 'C': OCR1A=C7;
                              break;
                    case 'D': if(flat){OCR1A=Db7;}
                                  else {OCR1A=D7;}
                                  break;
                    case 'E': if(flat){OCR1A=Eb7;}
                                  else {OCR1A=E7;}
                                  break;
                    case 'F': OCR1A=F7;
                              break;
                    case 'G': if(flat){OCR1A=Gb7;}
                                  else {OCR1A=G7;}
                                  break;
                } 
                break;
        case 8: switch (note) {
                    case 'A': if(flat){OCR1A=Ab8;}
                                  else {OCR1A=A8;}
                                  break;
                    case 'B': if(flat){OCR1A=Bb8;}
                                  else {OCR1A=B8;}
                                  break;
                    case 'C': OCR1A=C8;
                              break;
                    case 'D': if(flat){OCR1A=Db8;}
                                  else {OCR1A=D8;}
                                  break;
                    case 'E': if(flat){OCR1A=Eb8;}
                                  else {OCR1A=E8;}
                                  break;
                    case 'F': OCR1A=F8;
                              break;
                    case 'G': if(flat){OCR1A=Gb8;}
                                  else {OCR1A=G8;}
                                  break;
                } 
                break;
        default: OCR1A=0x0000;
    }
}

void music_off(void) 
{
    //this turns the alarm timer off
    notes=0;
    TCCR1B &= ~((1<<CS11)|(1<<CS10));
    //and mutes the output
    PORTD |= mute;
}

void music_on(void) 
{
    //this starts the alarm timer running
    notes=0;
    TCCR1B |= (1<<CS11)|(1<<CS10);
    //unmutes the output
    PORTD &= unmute;
    //and starts the selected song
    play_song(song, notes);
}

void music_init(void) 
{
    //initially turned off (use music_on() to turn on)
    TIMSK |= (1<<OCIE1A);  //enable timer interrupt 1 on compare
    TCCR1A = 0x00;         //TCNT1, normal port operation
    TCCR1B |= (1<<WGM12);  //CTC, OCR1A = top, clk/64 (250kHz)
    TCCR1C = 0x00;         //no forced compare
    OCR1A = 0x0031;        //(use to vary alarm frequency)
    music_off();
    beat = 0;
    max_beat = 0;
    notes = 0;
    song = 1;
} 

/*********************************************************************/
/*                             TIMER1_COMPA                          */
/*Oscillates pin7, PORTD for alarm tone output                       */
/*********************************************************************/

ISR(TIMER1_COMPA_vect) 
{
    PORTD ^= ALARM_PIN;         //flips the bit, creating a tone
    if(beat >= max_beat)        //if we've played the note long enough
    {
        notes++;                //move on to the next note
        play_song(song, notes); //and play it
    }
}
