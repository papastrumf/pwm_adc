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
//#include <SoftwareSerial.h>
#define VTRG 158       // 680 / 1100 * 1024 [mV]

int led = 1;           // the pin that the LED is attached to
int pin = 0;           // /OC1A
int napon;             // ocitana vrijednost
int PWM1;              // ovo ide u PWM reg.
int br;                // counter
int ad1 = 2;
//int Rx = 3, Tx = 4;    // serial pins
//SoftwareSerial seriski(Rx, Tx);

// the setup routine runs once when you press reset:
void setup()  { 
  // declare pin 0 to be an output:
//  pinMode(led, OUTPUT);
  pinMode(pin, OUTPUT);
  pinMode(ad1, INPUT);
  
  for(br=0; br<2; br++) {
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(200);
  }
  br = 0;
//  randomSeed(millis());
//  TCCR0B = 0<<WGM02 | 1<<CS01;    // za pin1 TC-0
//  TCCR1 = 0<<PWM1A | 0<<COM1A0 | 1<<CS10;
//  GTCCR = 1<<PWM1B | 2<<COM1B0;
  TCNT1 = 0;
  TCCR1 = 0;
  GTCCR = 1<<PSR1;
/*
  PLLCSR |= (1 << PLLE);           //Start PLL
  while( !(PLLCSR & (1<<PLOCK)) ); //Wait for PLL lock
  //PLLCSR |= (1<<LSM );           //Low Speed PLL that clocks 32Mhz, not 64Mhz
  PLLCSR |= (1 << PCKE);           //Enable PLL  
*/
  TCCR1 = 1<<PWM1A | 1<<COM1A0 | 1<<CS10;
  OCR1A = 0x3F;
  OCR1C = 0x7F;
  PWM1 = 0x3F;
  
//  seriski.begin(9600);
  ADMUX = 0xA1;
  ADCSRA = 0x87;
} 

// the loop routine runs over and over again forever:
void loop()  { 
  // set the brightness of pin:
//  OCR1A = analogRead(ad1) / 8;
//  seriski.print(millis());
//  seriski.print(": ");
//  seriski.println(napon);
/*
  if (OCR1A == 0x30)
    OCR1A = 0x50;
  else
    OCR1A = 0x10;
*/
//  analogWrite(led, OCR1A);
  ADCSRA |= 0x40;
//  delay(10);
//  OCR1A = ADCH >> 1;
  napon = (int)ADCH;
  if(napon - VTRG > 4) {
    PWM1 -= 4;
    OCR1A = PWM1;
  } else if(VTRG - napon > 4) {
    PWM1 += 4;
    OCR1A = PWM1;
  }
  if(PWM1 < 0 || PWM1 > 127) {
    PWM1 = napon >> 1;
    OCR1A = PWM1;
  }
  
  // wait    
  delay(500);
}
