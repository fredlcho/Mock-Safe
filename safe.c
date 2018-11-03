#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define LED_PIN0 (1 << PB0) //green led input
#define LED_PIN1 (1 << PB1) //yellow/blue led input
#define LED_PIN2 (1 << PB2) //red led input
#define SWITCH1 (1 << PB3) //key 1 switch
#define SWITCH0 (1 << PB4) //Key 0 switch
#define lowdelay 400
#define verylowdelay 1
#define ddrbgbconst 0b00000101 //works for blue + green, blue + yellow, green + red, red + yellow,
#define portbgbconst 0b00000011
#define portbbrconst 0b00000110//works for red + yellow
#define portbyellowflashconst 0b00000010
#define arraysize 6
#define flashcount 8

typedef enum{
  Initialization_State,
  Locked_State,
  Incorrect_State,
}SafeState;

typedef enum{RELEASED = 0, PRESSED} key_t;

SafeState mystate = Initialization_State;
key_t keystate0 = RELEASED;
key_t keystate1 = RELEASED;

volatile uint8_t input = 0b00000000;
uint8_t safecode;
volatile int presscount = 0;
int myarray[arraysize];
int safearray[arraysize];

//debounce stuff
volatile uint8_t history0 = 0;
volatile uint8_t history1 = 0;
volatile int boolean0 = 1; //initially true;
volatile int boolean1 = 1;

//timer0 function will be checking pin every 5ms
static inline void initTimer0(void){
  // TCCR0A |= (1 << WGM01);              //clears timer counter on match
  TCCR0B |= (1 << CS01) | (1 << CS00); //64 prescale
  TIMSK |= (1 << OCIE0A);              //enables timer ISR when clock matches
  OCR0A = 78;                          // 64 prescale & 78 = 5ms
}

void lightyellow(void){
  DDRB = LED_PIN0|LED_PIN1;
  PORTB = LED_PIN1;
}

void lightred(void){
  DDRB = LED_PIN1|LED_PIN2;
  PORTB = LED_PIN2;
}

void lightblue(void){
  DDRB = LED_PIN1|LED_PIN2;
  PORTB = LED_PIN1;
}

int arraycompare(int a[],int b[]){
  for(int i = 0;i<=arraysize-1;++i){
    if(a[i] != b[i]){
      return 0;
    }
  }
  return 1;
}
ISR(TIMER0_COMPA_vect){
  history0 = history0 << 1;
  history1 = history1 << 1;

  if ((PINB & SWITCH1) == 0) { // low if button is pressed!
    history1 = history1 | 0x1;
  }
  if ((PINB & SWITCH0) == 0){ //the second switch that gets debounced DOES NOT WORK
    history0 = history0 | 0x1;
  }
  if((history1 & 0b111111) == 0b111111){
    keystate1 = PRESSED;
  }
  if((history0 & 0b111111) == 0b111111){
    keystate0 = PRESSED;
  }
  if((history1 & 0b00111111) == 0){
    keystate1 = RELEASED;
    boolean1 = 1;
  }
  if((history0 & 0b00111111) == 0){
    keystate0 = RELEASED;
    boolean0 = 1;
  }
  if ((keystate1 == PRESSED) && boolean1)
    {
      myarray[presscount] = 1;
      presscount++;
      boolean1 = 0;
      lightblue();
      _delay_ms(1);
    }
  if((keystate0 == PRESSED) && boolean0)
    {
      myarray[presscount] = 0;
      presscount++;
      boolean0 = 0;
      lightblue();
      _delay_ms(1);
      // }
    }
}

void initializefunc(void){
  presscount = 0;
  while(presscount != arraysize){
    DDRB = LED_PIN0|LED_PIN1;
    PORTB = LED_PIN0;
    PORTB ^= portbgbconst;
  }
  presscount = 0;
  for(int i = 0;i<=arraysize-1;++i){
    safearray[i] = myarray[i];
    // _delay_ms(2000);
  }
  mystate = Locked_State;
}

void lockedfunc(void){
  
  presscount = 0;
  while(presscount != arraysize){
    DDRB = LED_PIN1|LED_PIN2;
    PORTB = LED_PIN2;
  }
  if(arraycompare(safearray,myarray)){
    presscount = 0;
    mystate = Initialization_State;
  }else{
    mystate = Incorrect_State;
    // presscount = 0;
  }
}

void incorrectfunc(){
  int mycount = 0;
  lightyellow();
  while(mycount<flashcount){
    PORTB ^= portbyellowflashconst;
    _delay_ms(lowdelay);
    mycount+=1;
  }
  presscount = 0;
  mystate = Locked_State;
}


void main(void){
  sei();
  initTimer0();
  // sei();
  for(int i = 0;i<=arraysize-1;++i){
    safearray[i] = 0;
  }
  while(1){
    if(mystate == Initialization_State){
      initializefunc(); //TODO: define initialization state stuff
    }
    if(mystate == Locked_State){
      lockedfunc(); //TODO: define locked state stuff
    }
    if (mystate == Incorrect_State){
      incorrectfunc(); //TODO :define incorrect state stuff
    }
  }
}
