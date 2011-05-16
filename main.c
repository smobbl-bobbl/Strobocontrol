// +---------------------------------------------------------------------------+
// | Programm......: LEDcontrol.c                                              |
// +---------------------------------------------------------------------------+
// | Projekt.......: LED.PRJ                                                   |
// +---------------------------------------------------------------------------+
// | System........: MSP430F235T                                               |
// +---------------------------------------------------------------------------+
// | Entwicklungs-Plattform..... : Win7 mit IAR_R5.20.1                        |
// +---------------------------------------------------------------------------+
// | Autor.........: S.Fath                                                    |
// +---------------------------------------------------------------------------+
// | Stand.........: 01.04.2011 / Version 0.1 / Bobbl                          |
// |                                                                           |
// |                 TT.MM.JJJJ / Version X.YZ                                 |
// |                                                                           |
// +---------------------------------------------------------------------------+
// | Beschreibung..:                                                           |
// | Version_1:                                                                |
// |                                                                           |
// |                                                                           |
// +---------------------------------------------------------------------------+
// | Benötigt......:                                                           |
// +---------------------------------------------------------------------------+
// | Hardware I/O..: SHDN                                                      |
// |                   Port P1.0 => SHDN freqency filter                       |
// |                   Port P1.1 => D-Sub3 => SHDN ICs LED board               |
// |                   Port P1.2 =>                                            |
// |                 LEDs                                                      |
// |                   Port P1.3 => D-Sub6 => LED IC - PWM LED 3               |
// |                   Port P1.4 =>                                            |
// |                   Port P1.5 => D-Sub7 => LED IC - PWM LED 4               |
// |                   Port P1.6 => D-Sub8 => LED IC - PWM LED 1               |
// |                   Port P1.7 => D-Sub9 => LED IC - PWM LED 2               |
// |                                                                           |
// |                 --> Port1: FF                                                          |
// |                                                                           |
// |                 other                                                     |
// |                   Port P6.0 => lower poti                                 |
// |                   Port P6.1 => upper poti                                 |
// |                   Port P6.7 => audio IN                                   |
// |                                                                           |
// |                                                                           |
// |                                                                           |
// |                                                                           |
// +---------------------------------------------------------------------------+
// |                                                                           |
// |                                                                           |
// +---------------------------------------------------------------------------+
//
// Programm.......: xxx Byte
// Konstanten.....:  40 Byte
// Daten / Var....:   8 Byte
//



// #include "io430.h"
#include "msp430x23x.h"

#define LED1 0x40 //LED-Strang 1
#define LED2 0x80
#define LED3 0x08
#define LED4 0x20
#define LED_PWR_ON 0x02 //SHDN powersupply of led board
#define FREQ_ON 0x01 //SHDN frequency filter

#define AUDIO 0x80


void INIT_PORTS(void);
void INIT_ADC(void);

void init_XT2(void);
void init_TimerA(unsigned int cycles);
void init_TimerB(unsigned int cycles);
void init_PWM_TimerA(void);
void set_PWM_duty_cycle(unsigned int duty);
void timerA_init(void);
void timerA_start(int timing[2], int mode);

unsigned int i = 0;
unsigned int intDelay = 2;
unsigned char delay = 0x01;

// +---------------------------------------------------------------------------+

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
//  timerA_start(timing, OUTMOD_SET_RESET); // starte PWM
  INIT_PORTS();
//  INIT_ADC();
//  init_XT2();
  init_PWM_TimerA();
  set_PWM_duty_cycle(0);
  P1OUT |= LED_PWR_ON;
  P1OUT |= LED2;
  //_BIS_SR(LPM3_bits);
  _BIS_SR(LPM0_bits + GIE);                 // Enter LPM0 w/ interrupt

//      while(1)                      // warte bis sich was tut
//    {
//    }
//  return 0;
}

// +---------------------------------------------------------------------------+

  // INIT HARDWARE

// +---------------------------------------------------------------------------+

  // Initialisierung der Ports

void INIT_PORTS(void)
{
  P1DIR = 0xFF;	//Port 1 initialisieren als Ausgänge
  P1SEL = 0x00; //keine special functions
  P1IE  = 0x00; //kein Interrupt
  P1OUT = 0x00; //erstmal alle ausgänge auf "low"
  
  P6DIR = 0xFF; //Ports erstmal als ausgang
  P6SEL = 0x00; //alles erstmal als binärsignal
//  P6IE  = 0x00; //nicht interrupt, sondern regelmäßige abfrage (analogsignal!)
  
  
}


  // Initialisierung des ADC
  
void INIT_ADC(void)
{
  ADC12CTL0 = ADC12ON;
  P6DIR &= ~0x83; //Port 7, 1 und 0 definiert als eingang, rest ist ohnehin nicht belegt, daher als ausgang
  P6SEL |= 0x83; //alle eingänge sind mit ADCs verknüpft
}

unsigned int sampling_ADC(void)
{
  ADC12CTL0 |= ADC12SC + ENC;   // Sampling open
  ADC12CTL0 &= ~ADC12SC;        // Sampling closed, start conversion
  while ((ADC12CTL1 & ADC12BUSY) == 1);   // ADC12BUSY?
  return(ADC12MEM0);	// return the value read from ADC P6.0
}

  // Initialisierung der Quarze 

void init_XT2(void)
{
  unsigned int i;
  WDTCTL = WDTPW + WDTHOLD;             // Stop WDT
  BCSCTL1 &= ~XT2OFF;                   // XT2 = HF XTAL
  do 
  {
    IFG1 &= ~OFIFG;                       // Clear OSCFault flag
    for (i = 0xFF; i > 0; i--);           // Time for flag to set
  }
  while ((IFG1 & OFIFG) != 0);          // OSCFault flag still set?                
  BCSCTL2 |= SELM1;                     // MCLK = XT2 (safe)
}

// +---------------------------------------------------------------------------+
/*

 // Initialisierung des Timers A 

void init_TimerA(unsigned int cycles)
{
 TACTL = TASSEL1 + TACLR;              // SMCLK, clear TAR
 CCTL0 = CCIE;                         // CCR0 interrupt enabled
 CCR0 = cycles;
 TACTL |= MC_2;                         // Start Timer_A in continuous mode
 _EINT();                              // interrupt enable
}


// Timer A0 interrupt service routine
interrupt (TIMERA0_VECTOR) Timer_A(void)
{
 P1OUT ^= 0x01;                        // Toggle P1.0
 CCR0 += 50000;                        // Add Offset to CCR0
}


// +---------------------------------------------------------------------------+

 // Initialisierung des Timers B

void init_TimerB(unsigned int cycles)
{
 TBCTL = TBSSEL1 + TBCLR;              // SMCLK, clear TAR
 TBCCTL0 = CCIE;                       // CCR0 interrupt enabled
 TBCCR0 = cycles;
 TBCTL |= MC_2;                         // Start Timer_B in continuous mode
 _EINT();                              // interrupt enable
}

// Timer B0 interrupt service routine
interrupt (TIMERB0_VECTOR) Timer_B(void)
{
 P1OUT ^= 0x01;                        // Toggle P1.0
 TBCCR0 += 50000;                      // Add Offset to CCR0
}


// +---------------------------------------------------------------------------+
*/
 // Initialisierung von PWM

void init_PWM_TimerA(void)
{
 TACCR0 = 32768 - 1;                         // PWM Period 64 --> 512 Hz

//  TACCTL0 = OUTMOD_7;                     // CCR1 reset/set
 TACCTL1 = OUTMOD_7;
 P1DIR |= LED1;                        // P1.2 PWM output
 P1SEL |= LED1;                        // P1.2 and TA1/2 otions
 
 TACCTL2 = OUTMOD_7;
 P1DIR |= LED3;                        // P1.2 PWM output
 P1SEL |= LED3;   

                     // P1.2 and TA1/2 otions
// TACTL |= MC0;                         // Start Timer_A in up mode
 TACTL = TASSEL_1 + MC_1 + ID_3;

//lasse interrupt von TACCTL0 zu
 
 TACCTL0 |= CCIE;
  P1DIR &= ~LED4;	//Port 1.5 initialisieren als Eingang
//  P1SEL = 0x00; //keine special functions
  P1IES = 0x00;
  P1IE  |= LED4; //kein Interrupt
  P1IFG = 0x00;
}

void set_PWM_duty_cycle(unsigned int duty)
{
 TACCR1 = duty;                           // CCR1 PWM duty cycle 
 TACCR2 = TACCR1 >> 1;
// TACCR2 = TACCR1 - 12;
}



// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
  if (intDelay == 0) {
    delay = 0x00;
    TACCR0 = 0x3F;
  }
  if (delay == 0x00) {
    if (TACCR1 < 0x2F) {
      i++;
      if (i >= 0x9FF >> 2) {
        set_PWM_duty_cycle(TACCR1 + 1);
        P1IFG = 0x00;
        i = 0x00;
	  }
    }
	else {
	  TACCTL0 &= ~CCIE;
	}
  }
  else {
    intDelay = intDelay - 1;
  }
}


// +---------------------------------------------------------------------------+

 // SOFTWARE

// +---------------------------------------------------------------------------+


 // PWM

