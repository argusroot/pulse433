
#include "pulse433.h"
#include <util/delay.h> // para _delay_us
#include "pin.h"

Pulse433 *pulse;

int main(void)
{
	Pin *pin_tx = new Pin(&PORTB, &DDRB, &PINB, 5, 0); // 13
	Pin *pin_rx = new Pin(&PORTD, &DDRD, &PIND, 7, 0); // 7

	int tx = false;

	if(tx) {
		pulse = new Pulse433(pin_tx, 1);
		uint8_t msg1[]={0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x11,0x22};
		while(1) {
			  //pulse->waitMillis(1000);
			  _delay_ms(1000);
			  pulse->sendMsg(msg1);
		}
	}
	else {
		pulse = new Pulse433(pin_rx, 0);
		char msg[8];
		while(1) {
			//pulse->interruptHandler();
			// _delay_ms(500);
			// sprintf(msg,"%d ", pulse->lastPulse);
			// pulse->uartPutString(msg);
		}
	}
}

// trata interrupções no pino de RX
ISR(PCINT2_vect) {
	pulse->interruptHandler();
}
