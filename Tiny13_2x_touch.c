/*
 * mega8_2xtouch.c
 *
 * Created: 2013-08-25 18:01:44
 *  Author: Olek
 *  MCU: ATTINY13A
 *  LED = TP.0 to ground
 *  TOUCH1 = PortD.2
 *  TOUCH2 = PortD.3
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>

//TP
#define	TP	PORTB
#define TD	DDRB
#define TI	PINB
#define LEDPIN1	3
#define LEDPIN2	4
#define TOUCH1	2
#define TOUCH2	0
#define TRIG	1

uint32_t cal1=0, cal2=0;

void init(void);
uint32_t measure_cap(uint8_t pin);
uint32_t calibrate(uint8_t pin);

#define NOOP asm volatile("nop" ::)

int main(void)
{
	init();
	cal1 = calibrate(TOUCH1);
	cal2 = calibrate(TOUCH2);

    while(1)
    {
		if (measure_cap(TOUCH1)>(cal1+2))
		{
			TP |= (1<<LEDPIN1);
		} 
		else
		{
			TP &= ~(1<<LEDPIN1);
		}
		
		if (measure_cap(TOUCH2)>(cal2+2))
		{
			TP |= (1<<LEDPIN2);
		} 
		else
		{
			TP &= ~(1<<LEDPIN2);
		}
		_delay_ms(2);
	}
}

void init(void)
{
	TD = (1<<LEDPIN1)|(1<<LEDPIN2)|(1<<TRIG);	//outputs
	TP = 0x00;
	//SFIOR |= (1<<PUD);
	MCUCR |= (1<<PUD);
}

uint32_t measure_cap(uint8_t pin)
{
	uint32_t i=0;
	while(TP & (1<<pin));	//wait for discharge sensor pin
	TD |= (1<<pin);			//set sensor pin as output, dicharge it to end
	NOOP;
	NOOP;
	NOOP;
	TD &= ~(1<<TRIG);		//set "charger" as input, i third state (as disconnected)
	TD &= ~(1<<pin);		//set sensor pin as input,
	TD |= (1<<TRIG);		//now "plug in the charger"
	TP |= (1<<TRIG);		//start charging sensor
	do
	{
		i++;				//increment counter
	} while (!(TI & (1<<pin)));	//if the pin is in high state
	TP &= ~(1<<TRIG);	//stop charging
	return i;
}

uint32_t calibrate(uint8_t pin)
{
	uint32_t max = 0; //max measured capacity value
	uint32_t ptemp = 0;	//momentary measured capacity value
	for (uint8_t p=0;p<100;p++)
	{
		ptemp = measure_cap(pin);
		if (ptemp>max)
		{
			max = ptemp;
		}
	}
	return max;
}