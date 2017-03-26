#ifndef __PULSE433_H_INCLUDED__
#define __PULSE433_H_INCLUDED__

#define F_CPU 16000000UL // 16 MHz
#define BAUD 9600

// for uint_t types
#include <stdint.h>
#include <stdio.h> //sprintf
#include <string.h> //array de ids
#include <stdlib.h> // malloc
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h> // para atomic
#include <util/setbaud.h> // uart
#include <util/delay.h> // para _delay_us
#include "pin.h"

class Pulse433
{

private:
	uint8_t isTx;
	Pin *pin;

	// data control
	uint8_t data_n;
	uint8_t data_bit_n;
	uint8_t data[8];

	// time
	uint8_t tick;
	void resetTimer();
	uint8_t getMicros();

public:
	int lastPulse;
	// construtor
	Pulse433(Pin*, uint8_t);

	// rx tx
	void sendMsg(uint8_t *);
	void sendPreamble();
	void sendByte(uint8_t byte);
	void interruptHandler();

	// time
	void waitMicros(uint32_t);
	void waitMillis(uint32_t);

	// USART
	void uartInit();
	void uartPutChar(char);
	void uartPutString(char*);
	char uartGetchar();
};

#endif // __PULSE433_H_INCLUDED__
