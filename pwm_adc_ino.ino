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
#define NUZR 32

//int pwm = 0;           // /OC1A - ATtiny85
int pwm = 11;          // OC2A - ATmega328
int napon1;            // ocitana vrijednost
int napon2;
int br;                // counter
//int ad1 = 2, ad2 = 4;  // ATtiny85
int ad1 = 15, ad2 = 16;  // ATmega328
int PWM1;              // ovo ide u PWM reg.
int PWM2;              // prijasnja vrijednost PWM1
int rst_st = 0;
int esp_rst = 3;
int slp_st = 0;
int pwm12[NUZR +1][2];
int delta;

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

// ATtiny85  //
/*  TCNT1 = 0;
  TCCR1 = 0;
  GTCCR = 1<<PSR1;
  TCCR1 = 1<<PWM1A | 1<<COM1A0 | 1<<CS10;
  OCR1A = 0x3F;
  OCR1C = 0x7F;
  PWM1 = 0x3F;  */
// ATmega328  //
  TCNT2 = 0;
  TCCR2A = 1<<COM2A1 | 1<<COM2A0 | 1<<WGM21 | 1<<WGM20;
  TCCR2B = 1<<CS20;
  OCR2A = 0x7F;
  PWM1 = 0x7F;

//  ADMUX = 0xA1;  // ATtiny85
  ADCSRA = 1<<ADEN | 1<<ADPS2;  // ATmega328 - 0x84
  ADMUX = 1<<REFS1 | 1<<REFS0 | 1<<ADLAR | 1<<MUX0;  // 0xE1

  Serial.begin(115200);
  Serial.println("ola!");
  delay(100);
  br=0;
  delta = 0;
}

// the loop routine runs over and over again forever:
void loop() {
//  ADMUX = 0xA1;  // ATtiny85
  ADMUX = 0xE1;  // ATmega328
//  delay(5);
  ADCSRA |= (1<<ADSC);
  while(ADCSRA & (1<<ADSC))  // ADSC
    Serial.print(".");
  napon1 = (int)ADCH;

  ADMUX = 0xE2;  // ATmega328
//  delay(5);
  ADCSRA |= 0x40;
  while(ADCSRA & (1<<ADSC))  // ADSC
    Serial.print("-");
  napon2 = (int)ADCH;

  PWM2 = PWM1;
  if(napon1 > VTRG) {
    if(napon1 - VTRG > 12 && PWM1 > 9)  PWM1 -= 8;
    else  PWM1--;
//    PWM1--;
  } else if(VTRG > napon1) {
    if(VTRG - napon1 > 48 && PWM1 < 150)  PWM1 += 80;
    else if(VTRG - napon1 > 12 && PWM1 < 245)  PWM1 += 8;
    else  PWM1++;
//    PWM1++;
/*
  } else if(!rst_st) {
    rst_st = 1;
    digitalWrite(esp_rst, HIGH);
*/
  }

  pwm12[br % NUZR][0] = napon1;
  pwm12[br % NUZR][1] = napon2;
  pwm12[NUZR][0] = 0;
  pwm12[NUZR][1] = 0;
  for(int i=0; i<NUZR; i++) {
    pwm12[NUZR][0] += pwm12[i][0];
    pwm12[NUZR][1] += pwm12[i][1];
  }
  pwm12[NUZR][0] /= NUZR;
  pwm12[NUZR][1] /= NUZR;
  br++;
  if(br == 8192)  br=NUZR *2;
  
//  OCR1A = PWM1;  //  ATtiny85
  OCR2A = PWM1;  //  ATmega328
  Serial.print("PWM: ");
  Serial.print(PWM1);
  Serial.print(", A1: ");
  Serial.print(napon1);
//  if(PWM1 < 0 || PWM1 > 127) {
  if(PWM1 <= 2 || PWM1 >= 252) {
    PWM1 = PWM2;
//    OCR1A = PWM1;  //  ATtiny85
    OCR2A = PWM1;  //  ATmega328
  }
  Serial.print(", A2: ");
  Serial.print(napon2);
  Serial.print(", dif: ");
  Serial.println(pwm12[NUZR][1] - pwm12[NUZR][0]);
/*
  Serial.print(", ave: ");
  Serial.print(pwm12[NUZR][0]);
  Serial.print("-");
  Serial.print(pwm12[NUZR][1]);
  Serial.print(", br: ");
  Serial.println(br);
*/
  
  if(br == 5 * NUZR)  delta = pwm12[NUZR][1] - pwm12[NUZR][0];
  if(delta > 0 && (pwm12[NUZR][1] - pwm12[NUZR][0] < delta *3 /5) && !slp_st) {
    slp_st = 1;
    Serial.println(" **** pocni odbrojavanje ****");
  }
  if(delta > 0 && (pwm12[NUZR][1] - pwm12[NUZR][0] > delta *4 /5) && slp_st) {
    slp_st = 0;
    Serial.println(" ******* budjenje *******");
  }
  delay(20);
}

