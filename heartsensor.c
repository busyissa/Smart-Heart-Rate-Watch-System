#define F_CPU 16000000		
#define BAUD  9600

#include <avr/io.h>
#include <stdint.h>			
#include <avr/interrupt.h>
#include <util/delay.h>		



#define VOLTTHRESHOLD 600  // ~2.5V with Vcc=5V // adjust threshold as needed when new sensor arrives

//function prototypes
void initTimer1();
void initUART();
void putChar(unsigned char c);
void putString(unsigned char c[]);
void initADC();
unsigned int readADC(unsigned char channel);

volatile uint8_t seconds = 0;
volatile uint8_t tenSecFlag = 0;

//design : 

// heart rate sensor will have 3 Ports:
// 1. power
// 2. ground
// 3. data output

// Data will be analog, test with oscilloscope first,
// make sure to isolate it from external noise


// read for 10 seconds, and BPM = (number of beats in 10 seconds) * 6

// Convert analog data to digital using ADC



int main(void){

    //inits 
    initADC();      
    initUART();
    initTimer1();

    sei();

    uint8_t prev_above = 0; // track if previous reading was above threshold
    unsigned int bpmCalc = 0;


    while(1){
        //read ADC value from channel 0 (PINB5)
        unsigned int value = readADC(0);    
        uint8_t above = (value > VOLTTHRESHOLD);

        putString("ADC Value: ");
        putChar((value / 1000) + '0'); // thousands digit
        putChar(((value / 100) % 10) + '0'); // hundreds digit
        putChar(((value / 10) % 10) + '0'); // tens digit
        putChar((value % 10) + '0'); // ones digit

        putChar('\r');
        putChar('\n');
        _delay_ms(20);
        
        //add if rising edge detected, timer 1 counts and ccalcs BPM

        //convert adcValue to bpm (this is a placeholder, actual conversion will depend on the sensor's characteristics)
      //  unsigned int bpm = value; // replace with actual conversion formula


      
          if(tenSecFlag) { // if 10 seconds have passed, reset BPM calculation
              unsigned int bpm = bpmCalc * 6; // calculate BPM based on beats counted in 10 seconds
              //display to uart
              putString("Heart Rate: ");
              putChar((bpm / 100) + '0'); // hundreds digit
              putChar(((bpm / 10) % 10) + '0'); //
              putChar((bpm % 10) + '0'); // ones digit
              putChar('\r');
              putChar('\n');


              bpmCalc = 0;
              tenSecFlag = 0; // reset flag for next 10 second interval
          }
        
        if ((value > VOLTTHRESHOLD) && !prev_above) {  // beat detected
            bpmCalc++;
         //   _delay_ms(550); // delay to avoid counting the same beat multiple times (since 100bpm is about every 600ms)

      }
      prev_above = above;

    }

}

//(mostly) example function snippets from prev assignemtns / harris example code

/* ************************************************* */
/*                  TIMER1 FUNCTIONS                 */
/* ************************************************* */

  void initTimer1() {
      TCCR1A = 0;
      TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC mode, prescaler 1024
      OCR1A = 15624;          // 16,000,000 / 1024 = 15,625 ticks/sec → compare at 15,624 (0-indexed)
      TIMSK1 |= (1 << OCIE1A); // enable compare match A interrupt
      TCNT1 = 0;
  }

  ISR(TIMER1_COMPA_vect) {
      seconds++;
      if (seconds >= 10) {
          tenSecFlag = 1;      // signal that 10 seconds have past
          seconds = 0;
      }
  }


//Notes: 
//16mhz / 1024 prescaler = 15,625 ticks per sec .... so 1 tick = 64us
// interval in ms = TCNT1 / 15.625 (since 15.625 ticks = 1 ms)

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
    while(!(UCSR0A & (1 << UDRE0))); // wait till ready
	UDR0 = c; // Send character to terminal
//	_delay_ms(200);
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
	
    ADCSRA |= (1 << ADATE); // Set ADC Auto Trigger Enable (ATE)
	ADCSRB = 0; // 0 for free running mode
	ADCSRA |= (1 << ADEN); //Enable the ADC
	ADCSRA |= (1 << ADSC); //Start the ADC conversion, keeps running automatically
}

unsigned int readADC(unsigned char channel) {
	// clear bottom 4 bits of ADMUX to wipe any prev data
	// and set new channel
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	// continuously returns the newest ADC
	return ADC;
}