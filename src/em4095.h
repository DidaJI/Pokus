#ifndef em4095_H
#define em4095_H

//#include <Arduino.h>
//#include <main.h>

#define DEMOD 18
#define SHD 17
#define CLK 16

extern unsigned char RFIDStr[55], CipHEX[12], CipHEXLast[12];
extern char CipHEXAscii[12];
extern unsigned char ReadTag, StartBit, Bit, LastBit, BitTmp, start_bit_count, seq_bit_no, Parita, i;
extern unsigned char RuseniAnt, Ruseni;
extern unsigned long int CteckaACK;
extern unsigned int bit_time_cnts, RuseniTime;				// counts number of timer cycles during a bit reception

extern bool RecCip0;
void EMInit();



#endif