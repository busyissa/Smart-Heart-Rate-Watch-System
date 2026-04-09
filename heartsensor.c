#define F_CPU 16000000		// Clock Speed
#define BAUD  9600

#include <avr/io.h>
#include <stdint.h>			// Needed for uint8_t
#include <avr/interrupt.h>
#include <util/delay.h>		// Required for _delay_ms()


//function prototypes
void initUART();
void putChar(unsigned char c);
void putString(unsigned char c[]);
void initADC();
unsigned int readADC(unsigned char channel);

//design : 

// heart rate sensor will have 3 Ports:
// 1. power
// 2. ground
// 3. data output

// Data will be analog, test with oscilloscope first,
// make sure to isolate it from external noise


// Convert analog data to digital using ADC



int main(void){

    // PINB5 will be input from heart rate sensor, connect to ADC0
    DDRB &= ~(1 << DDB5); // set PINB5 as input

    //inits 
    initADC();      
    initUART();


    while(1){
        //read ADC value from channel 0 (PINB5)
        unsigned int adcValue = readADC(0);

        //convert adcValue to bpm (this is a placeholder, actual conversion will depend on the sensor's characteristics)
        unsigned int bpm = adcValue; // replace with actual conversion formula

        //send bpm value over UART
        putString("Heart Rate: ");
        putChar((bpm / 100) + '0'); // hundreds digit
        putChar(((bpm / 10) % 10) + '0'); // tens digit
        putChar((bpm % 10) + '0'); // ones digit
        putChar('\r');
        putChar('\n');

        _delay_ms(1000); // delay for 1 second before next reading
    }

}


/* ************************************************* */
/*                  UART FUNCTIONS                   */
/* ************************************************* */

void initUART() {
	unsigned int baudrate;
	// Set baud rate: UBRR =
	// [F_CPU/(16*BAUD)] -1
	baudrate = ((F_CPU/16)/BAUD) - 1;
	UBRR0H = (unsigned char)(baudrate >> 8);
	UBRR0L = (unsigned char) baudrate;
	// Enable receiver and transmitter
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
	// Set data frame: 8-1-N
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}

void putChar(unsigned char c) {
	UDR0 = c; // Send character to terminal
	_delay_ms(200);
}

void putString(unsigned char c[]) {
	//UDR0 = c; // Send character to terminal
	int i = 0;
	//loop until null reached (end of string)
	while(c[i]!= '\0'){
		while(!(UCSR0A & (1 << UDRE0)));
		UDR0 = c[i];
		i++;
	}
}


/* ************************************************* */
/*                   ADC FUNCTIONS                   */
/* ************************************************* */

void initADC() {
	ADMUX = 0; // use ADC0
	ADMUX |= (1 << REFS0); // UseAVcc as the reference
	//ADMUX |= (1 << ADLAR); //Left-align for 8-bit resolution
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 128 prescale for 16 MHz
	//ADCSRA |= (1 << ADATE); // Set ADC Auto Trigger Enable (ATE)
	//ADCSRB = 0; // 0 for free running mode
	ADCSRA |= (1 << ADEN); //Enable the ADC
	//ADCSRA |= (1 << ADSC); //Start the ADC conversion
}

unsigned int readADC(unsigned char channel) {
	// clear bottom 4 bits of ADMUX to wipe any prev data
	// and set new channel
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

	// start analog to digital conversion
	ADCSRA |= (1 << ADSC);

	// wait for clear of ADSC bit 
	while (ADCSRA & (1 << ADSC));

	// return the final 10-bit value stored in the ADC data register
	return ADC;
}