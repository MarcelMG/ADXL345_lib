/*
 * ADXL345_lib.c
 *
 * Created: 04.06.2019 13:16:43
 * Author : Marcel
 */ 

#include "ADXL345.h"
#define F_CPU 3333333
#include <util/delay.h>

#include <stdio.h>
// use pin PB1 for UART TX
#define UART_TX_LOW() PORTB.OUTCLR=PIN1_bm
#define UART_TX_HIGH() PORTB.OUTSET=PIN1_bm
#define BAUD 9600
#define ERROR_COMP 0.0 // for clock freq. deviation error compensation, e.g. 0.05 = 5%, just try until it works :)
#define BAUD_DELAY_US 1000000UL*(1+ERROR_COMP)/BAUD
int dbg_print_char(char c, FILE* stream){
	c = ~c;
	UART_TX_LOW();            // start bit
	for( uint8_t i = 10; i; i-- ){        // 10 bits
		_delay_us(BAUD_DELAY_US);            // bit duration
		if( c & 1 ){
			UART_TX_LOW();        // data bit 0
			}else{
			UART_TX_HIGH();           // data bit 1 or stop bit
		}
		c >>= 1;
	}
	return 0;
}
FILE USART_stream = FDEV_SETUP_STREAM(dbg_print_char, NULL, _FDEV_SETUP_WRITE);

int main(void)
{
	stdout = &USART_stream;
	PORTB.DIRSET = PIN1_bm;	// TX-pin PB1 as output for soft-UART
	printf("start\n");
	if( !ADXL345_init() ){	// WORKS :)
		printf("init success\n");
	}else{
		printf("init error\n");
		return 1;
	}
    ADXL345_enterMeasurementMode();
	uint16_t x, y, z;
    while (1){
		ADXL345_getRawData(&x, &y, &z); // WORKS :)
		printf("x=\t%d\ty=\t%d\tz=\t%d\n", x, y, z);
		_delay_ms(300);
    }
}

