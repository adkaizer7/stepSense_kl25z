#include "mbed.h"

DigitalOut myled(LED1);

#define PRINT_PAYLOAD 0

Serial nrf(PTE22,PTE23);
#if PRINT_PAYLOAD
Serial pcprintf(USBTX,USBRX);
#else
Serial pc(USBTX,USBRX);
#endif
Serial xbee(PTE0,PTE1);

#define NUMBER_SAMPLES 1
#define NUMBER_BYTES_PER_SAMPLE 4

#if 0
int main() {
	int count = 0;
	int countMsg = 0;
	char msgToSend[NUMBER_SAMPLES * NUMBER_BYTES_PER_SAMPLE + 1];
	msgToSend[NUMBER_SAMPLES * NUMBER_BYTES_PER_SAMPLE] = '\0';

	nrf.baud(9600);
#if PRINT_PAYLOAD
	pcprintf.baud(9600);
#endif
	while(1) {
		if (xbee.readable()){
			int ch = xbee.getc();
#if PRINT_PAYLOAD
			pcprintf.putc(ch);
#else
			pc.putc(ch);
#endif

			if ((count >= 11) && (count <= 14))
			{
				msgToSend[countMsg] = (char)ch;
#if PRINT_PAYLOAD
				//pcprintf.printf("%c count%d\n",msgToSend[countMsg], count);
#endif
				countMsg++;
				if (countMsg == NUMBER_SAMPLES * NUMBER_BYTES_PER_SAMPLE){
					//nrf.printf("%s",msgToSend);
#if PRINT_PAYLOAD
					//pcprintf.printf("countMsg%d count %d **%s**",countMsg,count,msgToSend);
#endif
					countMsg = 0;
					myled = !myled;
				}
			}
			count++;
			if (count == 16){
				count = 0;
			}
		}
    }
}
#else
int main() {
	while(1) {
		if (xbee.readable()){
			int ch = xbee.getc();
			pc.putc(ch);
			nrf.putc(ch);
		}
	}
}
#endif
