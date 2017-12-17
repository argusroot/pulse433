#include "pulse433.h"

Pulse433::Pulse433(Pin *pin, uint8_t isTx){
	this->isTx = isTx;
	this->pin = pin;
	// seta a direção
	if(isTx) pin->output();
	else pin->input();
	// resetTimer();
	this->uartInit();

	// for arduino uno to run at 8Mhz
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		// CLKPR = 1 << CLKPCE;
		// CLKPR = 0b00000001; // prescaler 2
	}

	// habilita interrupção no pino digital 7
	cli();
	PCICR |= (1 << PCIE2);
  	PCMSK2 |= (1 << PCINT23);
	sei();
}

void Pulse433::sendMsg(uint8_t *data) {
	pin->write(1);
	waitMicros(400);
	pin->write(0);
	waitMillis(2);

	for(int i=0; i<3; i++) sendPreamble();
	for(int i=0; i<8; i++) {
		sendByte(data[i]);
	}
	pin->write(0);
}

void Pulse433::sendPreamble(){
	waitMicros(120);
	pin->write(1);
	waitMicros(100);
	pin->write(0);
}

void Pulse433::sendByte(uint8_t byte){
	for(uint8_t i=0; i<8; i++)
	{
		// 140 us 0 bit
		// 160 us 1 bit
		if((byte >> i) & 1 ) waitMicros(80);
		else waitMicros(60);
		pin->write(1);
		waitMicros(80);
		pin->write(0);
	}
}

char msg[100];
void Pulse433::interruptHandler(){

	if(pin->read()) return;

	// flow controll
	uint8_t valid_bit = false;
	uint8_t received_bit = 0;

	// if we had overflow at TIFR0 we assume 0us
	lastPulse = !(TIFR0 & (1 << TOV0)) ? getMicros() : 0;
	// now we can restart counting
	resetTimer();

	// low bit
	if(lastPulse > 140 && lastPulse < 160){ valid_bit = true; received_bit = 0;}
	// high bit
	else if(lastPulse > 160 && lastPulse < 180){ valid_bit = true; received_bit = 1;}
	// still getting preamble or noise
	else {
		valid_bit = false;
		data_n = 0;
		data_bit_n = 0;
	}

	if(valid_bit == true){
		// stores bit received
		//    received_bit ? (1<<data_bit_n) : 0;
		data[data_n] ^= (-received_bit ^ data[data_n]) & (1 << data_bit_n);
		// increments data counter
		if(++data_bit_n == 8){ ++data_n; data_bit_n = 0;}
		// received all 8 bytes
		if(data_n == 8) {
			data_n = 0;
			sprintf(msg,(char*)"\n data: 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X",
				data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
			uartPutString(msg);
			return;
		}
	}

}

void Pulse433::resetTimer(){
		TCCR0B = 0; // para timer
		TCNT0 = 0; // zera
		TIFR0 |= (1<<TOV0); // resets overflow flag
		TCCR0B |= (1 << CS01) | (1 << CS00) ; // 64 prescalar
}

uint16_t Pulse433::getMicros()
{
    uint16_t micros_return;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		micros_return = (TCNT0+2)*4;
    }
	return micros_return;
}

void Pulse433::waitMicros(uint32_t micros){
	uint16_t timer_max = 1024; // 16mhz, precalar 64, 4 us por tick
	while(micros >= timer_max){
		this->waitMicros(timer_max);
		micros -= timer_max;
	}
	this->resetTimer();
	// if not timeout AND not overflow
	while( (this->getMicros() < micros) &&  !(TIFR0 & (1 << TOV0)) );
}

void Pulse433::waitMillis(uint32_t millis){
	// to get 1000 micros
	for(int i=0; i<millis; i++){
		this->waitMicros(1000);
	}
}

/*********************************************************************
	USART utils
********************************************************************/
void Pulse433::uartInit() {
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

void Pulse433::uartPutChar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
    UDR0 = c;
}

void Pulse433::uartPutString(char* c) {
	while(*c){
		loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
	    UDR0 = *c++;
	}
}

char Pulse433::uartGetchar() {
	loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
    return UDR0;
}
