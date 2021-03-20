/* 
	This is the firmware for the fireflys which I am making Holly.

	they have a blue led on PB0 and an LDR on PB1

	In the day, the attiny85 will be put into powerdown mode while
	the firefly's battery charges. At night an interupt on PB1 will
	wake the firefly up and it will start pulsing at a random 
	interval until it is light again.

	For this project I will have to learn to use PWM (so the led
	can fade in and out) and pin interupts.
*/

#define F_CPU 1000000UL 
#include <avr/io.h> 
#include <util/delay.h>
#include <avr/interrupt.h> 
#include <avr/sleep.h>
#include <stdlib.h>



//Forward declarations
void pulse(void);
void setUp(void);

//This is the program entry point
int main(void){

	setUp();

	//Always sleep until interupted
	while(1){
		OCR0A = 255;
		sleep_mode();
	}

    // Execution should never reach this point.
	return 1;
}

//This interupt is called when PB2 goes low. (It keeps getting called over and over until PB2 goes high again)
ISR(INT0_vect)
{
	//Make sure the LED is completly dark
	OCR0A = 255;

	pulse();
	//Generate a random number between 50 and 350...
	int random = (rand() % 250)+50;

	//... wait for that amount of time ...
	while (--random > 0)
	{
		_delay_ms(100);
	}

	//... then pulse.
	

	OCR0A = 255;
	//Return to main routine.
	return;

}

//this fades the Led in and then out
void pulse(void){

	int random = (rand() % 10)+10;

	//Start the LED completly dim
	OCR0A = 255;

	//Fade it in by reducing the value of OCR0A
   	while((--OCR0A) > 0 )  _delay_ms(5); 

   	//Fade it out again
    while( (++OCR0A) < 200) _delay_ms(5);
    while( (++OCR0A) < 255) _delay_ms(10);//{ while ( (--random) > 0 ) _delay_ms(1); }; 
  

    //Make sure it's completly dark again
    OCR0A = 255;
	
   return;
}

//This is where everything is initialised so that the chip behaves the way I want it to.
void setUp(void){
	
	//Turn interupts off for setup so it isnt disturbed
	cli();


	//IO
    DDRB = (1 << PB0);  // set PB0 to be an output for the LED 
	DDRB &= ~(1 << PB2); // set PB2 to be an input
	PORTB = (1 << PB2); // Turn on the pull up resistor for PB2

	//PWM
	TCCR0A = (1 << COM0A1) | (1 << COM0A0) | (1 << WGM01) | (1 << WGM00); // Set up the timer for fast PWM
	TCCR0B = (1 << CS01); //set prescaler to 1:8 (whatever that means)
	OCR0A = 0; // Set the LED's initial state to dark
	
	//Interupts
	PCMSK = (1 << PCINT1); //Set PB1 as an interupt source
	MCUCR = 0; //Make interupt trigger on pin low
	GIMSK = (1 << 6); //Turn on the low level interupt


	//seed the random number generator with the contents of the ADC register (should be the reading from a noisy pin)
	ADMUX = (1 << REFS1); // Use the internal 1.1v voltage reference
	ADMUX |= (1 << ADLAR); //Make sure the result is left adjusted
	ADMUX |= (1 << MUX3) | (1 << MUX0); // set the analog source to the PB5

	ADCSRA = (1 << ADEN) | (1 << ADSC); //Turn on the ADC and start a reading

	while (ADCSRA & ( 1 << ADSC)){} //wait until the conversion is complete

	srand(ADCH); //Seed the random Number Generator
	
	ADCSRA &= ~(1 << ADEN); //Turn the ADC back off



    //Tell the chip that it should power down to sleep
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    //turn interupts back on
	sei();

	//exit setup
	return;
}


