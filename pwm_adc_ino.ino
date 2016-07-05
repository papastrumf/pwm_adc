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
#if defined(__AVR_ATmega328P__)
#define PWMpin   11     // OC2A - ATmega328
#define AD1pin   15     // ATmega328
#define AD2pin   16     // ATmega328
#else
#define PWMpin   0      // /OC1A - ATtiny85
#define AD1pin   2      // ATtiny85
#define AD2pin   4      // ATtiny85
#endif
#define RSTpin   5
#define WKUPpin  3
#define VTRG 163       // (3.3 * (2.7 / 12.7)) / 1.1 * 255 [mV]
#define NUZR 24
#if defined(__AVR_ATmega328P__)
#define PWM_MAX  255
#else
#define PWM_MAX  127
#endif

int napon1;            // ocitana vrijednost
int napon2;
int br;                // counter
int PWM1;              // ovo ide u PWM reg.
int PWM2;              // prijasnja vrijednost PWM1
int rst_st = 0;
int slp_st = 0;
int pwm12[NUZR +1][3];
int inic_delta;         // pocetna razlika napona (struja kroz R)
int inic_pwm;
long mls, mls_slp;

// the setup routine runs once when you press reset:
void setup() {
  // declare pin 0 to be an output:
  pinMode(PWMpin, OUTPUT);
  pinMode(AD1pin, INPUT);
  pinMode(AD2pin, INPUT);
  pinMode(RSTpin, OUTPUT);
  pinMode(WKUPpin, OUTPUT);
  delay(10);

//  digitalWrite(RSTpin, HIGH);
//  TCCR0B = 0<<WGM02 | 1<<CS01;    // za pin1 TC-0
//  TCCR1 = 0<<PWM1A | 0<<COM1A0 | 1<<CS10;
//  GTCCR = 1<<PWM1B | 2<<COM1B0;

// ATmega328  //
#if defined(__AVR_ATmega328P__)
  TCNT2 = 0;
  TCCR2A = 1<<COM2A1 | 1<<COM2A0 | 1<<WGM21 | 1<<WGM20;
  TCCR2B = 1<<CS20;
  OCR2A = 0x7F;
  PWM1 = 0x7F;
#else
// ATtiny85  //
  TCNT1 = 0;
  TCCR1 = 0;
  GTCCR = 1<<PSR1;
  TCCR1 = 1<<PWM1A | 1<<COM1A0 | 1<<CS10;
  OCR1A = 0x3F;
  OCR1C = 0x7F;
  PWM1 = 0x3F;
#endif

#if defined(__AVR_ATmega328P__)
  ADCSRA = 1<<ADEN | 1<<ADPS2;  // ATmega328 - 0x84
  ADMUX = 1<<REFS1 | 1<<REFS0 | 1<<ADLAR | 1<<MUX0;  // 0xE1
#else
  ADCSRA = 1<<ADEN | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0;  // ATtiny85 - 0x87
  ADMUX = 0xA1;  // ATtiny85
#endif

  digitalWrite(WKUPpin, HIGH);
#if defined(__AVR_ATmega328P__)
  Serial.begin(115200);
  Serial.println("ola!");
#endif

  mls = millis() + 55;
  while(millis() < mls)  ;
  br = 0;
  inic_delta = 0;
  inic_pwm = 0;
}


void loop() {
#if defined(__AVR_ATmega328P__)
  ADMUX = 0xE1;  // ATmega328
#else
  ADMUX = 0xA1;  // ATtiny85
#endif
  ADCSRA |= (1<<ADSC);
  while(ADCSRA & (1<<ADSC)) {  // ADSC
#if defined(__AVR_ATmega328P__)
    Serial.print(".");
#endif
    ;
  }
  napon1 = (int)ADCH;

#if defined(__AVR_ATmega328P__)
  ADMUX = 0xE2;  // ATmega328
  ADMUX = 0xA1;  // ATtiny85
#endif
  ADCSRA |= (1<<ADSC);
  while(ADCSRA & (1<<ADSC)) {  // ADSC
#if defined(__AVR_ATmega328P__)
    Serial.print("-");
#endif
    ;
  }
  napon2 = (int)ADCH;

  PWM2 = PWM1;
  if(napon1 > VTRG) {
#if defined(__AVR_ATmega328P__)
    if(napon1 - VTRG > 48 && PWM1 > 48)  PWM1 -= 40;
    else if(napon1 - VTRG > 12 && PWM1 > 9)  PWM1 -= 8;
#else
    if(napon1 - VTRG > 48 && PWM1 > 48)  PWM1 -= 40;
    else if(napon1 - VTRG > 12 && PWM1 > 9)  PWM1 -= 8;
#endif
    else  PWM1--;
  } else if(VTRG > napon1) {
#if defined(__AVR_ATmega328P__)
    if(VTRG - napon1 > 48 && PWM1 < 150)  PWM1 += 80;
    else if(VTRG - napon1 > 12 && PWM1 < 245)  PWM1 += 8;
#else
    if(VTRG - napon1 > 48 && PWM1 < 75)  PWM1 += 40;
    else if(VTRG - napon1 > 12 && PWM1 < 122)  PWM1 += 8;
#endif
    else  PWM1++;
/*
  } else if(!rst_st) {
    rst_st = 1;
    digitalWrite(RSTpin, LOW);
*/
  }

  pwm12[br % NUZR][0] = napon1;  // napon iza R
  pwm12[br % NUZR][1] = napon2;  // napon prije R
  pwm12[br % NUZR][2] = PWM1;    // sirina impulsa PWM (za DCDC)
  pwm12[NUZR][0] = 0;
  pwm12[NUZR][1] = 0;
  pwm12[NUZR][2] = 0;
  for(int i=0; i<NUZR; i++) {
    pwm12[NUZR][0] += pwm12[i][0];
    pwm12[NUZR][1] += pwm12[i][1];
    pwm12[NUZR][2] += pwm12[i][2];
  }
  pwm12[NUZR][0] /= NUZR;
  pwm12[NUZR][1] /= NUZR;
  pwm12[NUZR][2] /= NUZR;
  br++;
  if(br == 6144)  br=NUZR *8;    // vrati brojac da ne ode u minus
  
#if defined(__AVR_ATmega328P__)
  OCR2A = PWM1;  //  ATmega328
#else
  OCR1A = PWM1;  //  ATtiny85
#endif
#if defined(__AVR_ATmega328P__)
  Serial.print("PWM: ");
  Serial.print(PWM1);
  Serial.print(", A1: ");
  Serial.print(napon1);
#endif
#if defined(__AVR_ATmega328P__)
  if(PWM1 <= 2 || PWM1 >= 252) {    // ATmega328
#else
  if(PWM1 < 0 || PWM1 > 127) {    // ATtiny85
#endif
    PWM1 = PWM2;
#if defined(__AVR_ATmega328P__)
    OCR2A = PWM1;  //  ATmega328
#else
    OCR1A = PWM1;  //  ATtiny85
#endif
  }
#if defined(__AVR_ATmega328P__)
  Serial.print(", A2: ");
  Serial.print(napon2);
  Serial.print(", dif: ");
  Serial.print(pwm12[NUZR][1] - pwm12[NUZR][0]);
  if(slp_st)  Serial.println("  \\");
  else  Serial.println("  /");
/*
  Serial.print(", ave: ");
  Serial.print(pwm12[NUZR][0]);
  Serial.print("-");
  Serial.print(pwm12[NUZR][1]);
  Serial.print(", br: ");
  Serial.println(br);
*/
#endif
  
  if(br == 5 * NUZR) {
    inic_delta = pwm12[NUZR][1] - pwm12[NUZR][0] +1;
    inic_pwm = pwm12[NUZR][2] +1;
  }
//  if(inic_delta > 0 && (pwm12[NUZR][1] - pwm12[NUZR][0] < inic_delta *3 /5) && !slp_st) {
  if(inic_pwm > 0 && (pwm12[NUZR][2] < PWM_MAX /2) && !slp_st) {
    slp_st = 1;
#if defined(__AVR_ATmega328P__)
    Serial.print(" inic delta: ");
    Serial.println(inic_delta); 
    Serial.println(" **** pocni odbrojavanje ****");
#endif
    mls_slp = millis() +120000;
  }
//  if(inic_delta > 0 && (pwm12[NUZR][1] - pwm12[NUZR][0] > inic_delta *4 /5) && slp_st) {
  if(inic_pwm > 0 && (pwm12[NUZR][2] > PWM_MAX /2) && slp_st) {
    slp_st = 0;
#if defined(__AVR_ATmega328P__)
    Serial.println(" ******* budjenje *******");
#endif
  }
  if(slp_st && mls_slp <= millis()) {
    PWM1 += 12;
#if defined(__AVR_ATmega328P__)
    OCR2A = PWM1;  //  ATmega328
#else
    OCR1A = PWM1;  //  ATtiny85
#endif
    slp_st = 0;
#if defined(__AVR_ATmega328P__)
    Serial.println(" ******* timer 0 *******");
#endif
    digitalWrite(WKUPpin, LOW);    
    mls = millis() + 20;
    while(millis() < mls)  ;
    digitalWrite(WKUPpin, HIGH);
  } else {                // inace pricekaj
    mls = millis() + 20;
    while(millis() < mls)  ;
  }
}

