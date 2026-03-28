//
// Author: Sarah Harris
// Date: 3/4/2026
//
// Project: Example_keypad
//
// This program reads the keypad and outputs the number of the button pressed to the UART
// Board Row[1:4] = PD7:4 = Row 0:3
// Row[1] = PD7 = Row 0
// Row[2] = PD6 = Row 1
// Row[3] = PD5 = Row 2
// Row[4] = PD4 = Row 3
//
// Board Col[1:4] = PB0:3 = Col 0:3
// Col[4] = PB3 = Col 3
// Col[3] = PB2 = Col 2
// Col[2] = PB1 = Col 1
// Col[1] = PB0 = Col 0

// Board Col[1:4] = PD3:0 = Col 3:0
// Col[4] = PB3 = Col 3
// Col[3] = PB2 = Col 2
// Col[2] = PB1 = Col 1
// Col[1] = PB0 = Col 0

// Rows are (PD7:4) outputs from ATmega328p/pb
// Columns are inputs (PB3:0) to ATmega328p/pb with pull-up resistors.


#define F_CPU 16000000		// Clock Speed
#define BAUD  9600

#include <avr/io.h>
#include <stdint.h>			// Needed for uint8_t
#include <avr/interrupt.h>
#include <util/delay.h>		// Required for _delay_ms()

// Keypad access - allows changing the port easily
#define KEY_CDDR DDRB
#define KEY_RDDR DDRD
#define KEY_CPORT PORTB
#define KEY_RPORT PORTD
#define KEY_COLSREAD  PINB

void initIO();
void initUART();
unsigned char getChar();
void putChar(unsigned char c);
void printString(unsigned char *s);
void printInHex(uint8_t byte);
void printNibble(uint8_t nibble);

// Keypad functions
void checkAnyKeyPressed();
void debounce();
unsigned char identifyPressedKey();
void printPressedKey(unsigned char pressedKey);
void printRowandColumn(unsigned char row, unsigned char column);


int main(void)
{	unsigned char pressedKey;
	
	initIO();
	initUART();
	
	while (1) {
		checkAnyKeyPressed();
		debounce();
		pressedKey = identifyPressedKey();
		printPressedKey(pressedKey);
		_delay_ms(300);
	}
}

void checkAnyKeyPressed() {
	unsigned char anyKeyPressed;
	
	// Loop until a key is pressed
	do {
		// Ground all rows
		KEY_RPORT = 0x0;
		anyKeyPressed = KEY_COLSREAD & 0x0F;
	} while (anyKeyPressed == 0x0F);
}

void debounce() {
	// Wait and loop until get 2nd sample of key pressed
	do {
		_delay_ms(20);
	} while ( (KEY_COLSREAD & 0x0F) == 0x0F );
}

unsigned char identifyPressedKey() {
	unsigned char column, row; // Record column and row of pressed key
	unsigned char found = 0;   // Set to 1 when pressed key found
	
	// Array to hold keypad values
	unsigned char keypad[4][4] = {	{'0', '1', '2', '3'},
	{'4', '5', '6', '7'},
	{'8', '9', 'A', 'B'},
	{'C', 'D', 'E', 'F'}};

	// Check each row till find pressed key
	if (!found) {
		// Ground row 0
		KEY_RPORT = 0x70;
		_delay_ms(20);
		column = KEY_COLSREAD & 0xF;
		
		if (column != 0xF) {
			row = 0;
			found = 1;
		}		
	}
	
	if (!found) {
		// Ground row 1
		KEY_RPORT = 0xB0;
		_delay_ms(20);
		column = KEY_COLSREAD & 0xF;
		
		if (column != 0xF) {
			row = 1;
			found = 1;
		}
	}

	if (!found) {
		// Ground row 2
		KEY_RPORT = 0xD0;
		_delay_ms(20);
		column = KEY_COLSREAD & 0xF;
		if (column != 0xF) {
			row = 2;
			found = 1;
		}
	}

	if (!found) {
		// Ground row 3
		KEY_RPORT = 0xE0;
		_delay_ms(20);
		column = KEY_COLSREAD & 0xF;
		if (column != 0xF) {
			row = 3;
			found = 1;
		}
	}

	// Decode column
	switch (column) {
		case 0x0E:		column = 0; break;
		case 0x0D:		column = 1; break;
		case 0x0B:		column = 2; break;
		case 0x07:		column = 3; break;
		default:		column = 3;
	}
	
	// WprintRowandColumn(row, column);
	
	// Return value of key pressed
	return keypad[row][column];
}

void printPressedKey(unsigned char pressedKey) {
	unsigned char str[] = "The value pressed was: ";
	
	printString(str);
	putChar(pressedKey);
	putChar('\r');
	putChar('\n');
}

void printRowandColumn(unsigned char row, unsigned char column) {
	unsigned char rowstr[] = "Row = ";
	unsigned char colstr[] = "Col = ";
	
	printString(rowstr);
	printNibble(row);
	putChar('.'); putChar(' ');
	printString(colstr);
	printNibble(column);
	putChar('.'); putChar('\r'); putChar('\n');
}


void initIO() {

	KEY_RDDR |= 0xF0;	// PD7:4 = outputs (drive rows)
	KEY_CDDR &= 0xF0;	// PB3:0 = inputs (read columns)
	KEY_RPORT = 0xF0;	// Drive rows (PD7:4) to 1
	KEY_CPORT = 0x0F;	// Set pull-up resistors on columns (PB3:0)
}

void initUART() {
	unsigned int baudrate;

	// Set baud rate:  UBRR = [F_CPU/(16*BAUD)] - 1
	baudrate = ((F_CPU/16)/BAUD) - 1;
	UBRR0H = (unsigned char) (baudrate >> 8);
	UBRR0L = (unsigned char) baudrate;

	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);		// Enable receiver and transmitter
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);	// Set data frame: 8 data bits, 1 stop bit, no parity (8-1-N)
}

unsigned char getChar() {
	unsigned char c;
	
	while (!(UCSR0A & (1 << RXC0))) ;  // Loop until a character is typed
	c = UDR0;                          // Read typed character
	UCSR0A |= (1 << RXC0);             // Clear flag indicating that a character has been received

	return c;
}

void putChar(unsigned char c) {
	UDR0 = c;                          // Write typed character back to terminal
	while (!(UCSR0A & (1 << TXC0))) ;  // Loop until a character is completely transmitted back to terminal
	UCSR0A |= (1 << TXC0);             // Clear flag indicating that the character has transmitted
}

void printString(unsigned char *s) {
	unsigned char i = 0;
	
	while (s[i]) {
		putChar(s[i]);
		i++;
	}
}

void initTimer0() {
	TCCR0A |= (1 << COM0A1);;								// Non-inverted mode for OC0A
	TCCR0A |= (1 << WGM01) | (1 << WGM00);					// Fast PWM mode
	TCCR0B |= (1 << CS02) | (1 << CS00);					// Prescaler = 1024, so freq_Timer0 = 15.625 kHz
	OCR0A = 127;											// Initialize output compare register
	TCNT0 = 0;												// Initialize counter
}


void printInHex(uint8_t byte) {
	unsigned char nibble;
	
	nibble = (byte >> 4) & 0xF;		// nibble = most significant nibble (4 bits)
	printNibble(nibble);
	nibble = byte & 0xF;	// nibble = least significant nibble (4 bits)
	printNibble(nibble);
}


void printNibble(uint8_t nibble) {
	unsigned char printchar;
	
	if (nibble > 9) {					// Nibble is 0xA-F
		printchar = nibble + 55;
		putChar(printchar);
	}
	else {								// Nibble is 0-9
		printchar = nibble + 48;
		putChar(printchar);
	}
}


