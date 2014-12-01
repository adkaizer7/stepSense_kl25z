#include "mbed.h"

DigitalOut myled(LED1);

Serial nrf(PTE22,PTE23);
Serial pc(USBTX,USBRX);
Serial xbee(PTE0,PTE1);

int main() {

	while(1) {
		if (xbee.readable()){
			int ch = xbee.getc();
			pc.putc(ch);
			nrf.putc(ch);
			myled = !myled;
		}
    }
}
