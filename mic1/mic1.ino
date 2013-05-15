#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUDP.h>
#define SAMPLES 256

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 33, 33 };
byte server[] = { 192, 168, 33, 1 };
EthernetUDP clt;

const byte header[] = {0x53, 0x4E, 0x44};
byte buffer[2][SAMPLES*2];
unsigned int buffer_size = SAMPLES*2;
volatile static unsigned int i = 0, k = 0;
volatile byte processing;
volatile byte current;
volatile unsigned long start_sampling[2], end_sampling[2];

void setup() {
 
  // right adjusted
  ADMUX &= B11011111;
  ADMUX |= B01000000;
  ADMUX &= B11110000;

  ADCSRA |= B10000000;
  ADCSRA |= B00100000;
  ADCSRB &= B11111000;
  ADCSRA &= B11111000;
  //ADCSRA |= B00000111;
  ADCSRA |= B00000110;
  ADCSRA |= B00001000;
   
  Ethernet.begin(mac, ip);
  clt.begin(8888);

  processing = 0;
  current = 0;
  start_sampling[current] = micros();
  sei();
  ADCSRA |=B01000000;
}


void loop() {
  if(processing)
  {
    if(processing & 0x1)
      k = 0;
    else if(processing & 0x2)
      k = 1;
    clt.beginPacket(server, 8888);
    
    clt.write(header, 3);
    clt.write(k);  
    clt.write((start_sampling[k] >> 24) & 0xff);
    clt.write((start_sampling[k] >> 16) & 0xff);
    clt.write((start_sampling[k] >> 8) & 0xff);
    clt.write((start_sampling[k]) & 0xff);
    
    clt.write((end_sampling[k] >> 24) & 0xff);
    clt.write((end_sampling[k] >> 16) & 0xff);
    clt.write((end_sampling[k] >> 8) & 0xff);
    clt.write((end_sampling[k]) & 0xff);
    
    clt.write(buffer[k], buffer_size);
    clt.endPacket();
    
    processing = processing - (k+1) ;
  }
}

ISR(ADC_vect) {
  buffer[current][i] = ADCL;
  buffer[current][i+1] = ADCH;
  i +=2;
  if (i >= buffer_size)
  {
    i = 0;
    end_sampling[current] = micros();
    processing = processing | (current + 1); 
    current = (current + 1) % 2;
    start_sampling[current] = micros();
  }
}
