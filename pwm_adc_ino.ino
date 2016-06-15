//Attiny85 , running @ 8MHZ
//
//                           +-\/-+
//  Ain0       (D  5)  PB5  1|    |8   VCC
//  Ain3       (D  3)  PB3  2|    |7   PB2  (D  2)  INT0  Ain1
//  Ain2       (D  4)  PB4  3|    |6   PB1  (D  1)        pwm1
//                     GND  4|    |5   PB0  (D  0)        pwm0
//                           +----+

#define F_CPU 8000000  // This is used by delay.h library
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#define VTRG 163       // (3.3 * (2.7 / 12.7)) / 1.1 * 255 [mV]

int pwm = 0;           // /OC1A
int napon1;            // ocitana vrijednost
int napon2;
int br;                // counter
int ad1 = 2, ad2 = 4;
int PWM1;              // ovo ide u PWM reg.
int rst_st = 0;
int esp_rst = 3;

// the setup routine runs once when you press reset:
void setup() {
  // declare pin 0 to be an output:
  pinMode(pwm, OUTPUT);
  pinMode(ad1, INPUT);
  pinMode(ad2, INPUT);
  pinMode(esp_rst, OUTPUT);
  delay(10);

  digitalWrite(esp_rst, LOW);
//  TCCR0B = 0<<WGM02 | 1<<CS01;    // za pin1 TC-0
//  TCCR1 = 0<<PWM1A | 0<<COM1A0 | 1<<CS10;
//  GTCCR = 1<<PWM1B | 2<<COM1B0;
  TCNT1 = 0;
  TCCR1 = 0;
  GTCCR = 1<<PSR1;
  TCCR1 = 1<<PWM1A | 1<<COM1A0 | 1<<CS10;
  OCR1A = 0x3F;
  OCR1C = 0x7F;
  PWM1 = 0x3F;

  ADMUX = 0xA1;
  ADCSRA = 0x87;
}

// the loop routine runs over and over again forever:
void loop() {
  ADMUX = 0xA1;
  ADCSRA |= 0x40;
  napon1 = (int)ADCH;
  if(napon1 > VTRG) {
//    if(napon1 - VTRG > 8)  PWM1 -= 8;
//    else  PWM1--;
    PWM1--;
  } else if(VTRG > napon1) {
//    if(VTRG - napon1 > 8)  PWM1 += 8;
//    else  PWM1++;
    PWM1++;
/*
  } else if(!rst_st) {
    rst_st = 1;
    digitalWrite(esp_rst, HIGH);
*/
  }

  OCR1A = PWM1;
  if(PWM1 < 0 || PWM1 > 127) {
    PWM1 = napon1 >> 1;
    OCR1A = PWM1;
  }

  ADMUX = 0xA2;
  ADCSRA |= 0x40;
  napon2 = (int)ADCH;
  delay(10);
}

