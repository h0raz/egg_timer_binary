#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
//#include <stdint.h>

#define status GPIOR0				//allgemeines Datenregister
#define counterflag GPIOR1			//allgemeines Datenregister
#define count GPIOR2				//allgemeines Datenregister
#define SPWM_V 60					//PWM Teiler max 128!
#define SEKZORI 128/2               //Zeitteiler für eine Sekunde

#define ALARMFREQUENCY 20 //from 1(~8kHz) to 254(~33Hz) 8~1kHz
#define ALARMDUTY 16 //from 0(on) to sekz/2 = 31(almost of)

//#define delz 12						//= (uint8_t)((1000000/1)/(OCR1H*256+OCR1L)*10);
									// = (F_CPU /Prescaler)/Comparematch oder so ;)
//Input/Output
#define TIMELEDDDR DDRD
#define TIMESLEDDDR DDRB
#define ALARMDDR DDRA
#define TASTERDDR DDRB
#define TIMELEDGNDDDR DDRB

//Ports
#define TIMELEDPORT PORTD
#define TIMESLEDPORT PORTB
#define ALARMP PORTA
#define TASTERP PORTB
#define TIMELEDGNDP PORTB

//PinNr
#define SLED (1 << 3)

#define TIMELEDS 0x3f
#define TIMELED1GND (1 << 6)	//unten
#define TIMELED2GND (1 << 5)	//oben


#define ALARM1 (1 << 0)
#define ALARM2 (1 << 1)
#define TASTER (1 << 2)
#define DREH1 (1 << 1)
#define DREH2 (1 << 0)
#define DREH12 (DREH1 | DREH2)

//globale variablen
volatile uint8_t dstate;
register uint8_t count2 asm("r2");
register uint8_t count3 asm("r3");
register uint8_t buttonstate asm("r4");
register uint8_t sekz asm("r5");
register uint8_t alarmstate asm("r6");


uint8_t zust_state;
uint8_t zust_new_state;
uint8_t zust_count;
uint8_t button_state;
uint16_t button_count;

typedef struct {
	uint8_t s;
	uint8_t m;
	uint8_t h;
	}time_t;

time_t gtime;

//enums
enum {AUS, CHO, RUN, ALARM};
enum {NONE, NLL, L2, RES};
enum{BUT_NONE, BUT_PRESSED, BUT_RELEASED, BUT_SHORT, BUT_LONG, BUT_HOLD};


//Funktionen
uint8_t timeshow(uint8_t ltime);
void Zrpos(void);
void Zrneg(void);
void init(void);
void comp(void);
void statechange(uint8_t state);

//Interrupts
ISR(TIMER0_OVF_vect);
ISR(TIMER1_COMPA_vect);
