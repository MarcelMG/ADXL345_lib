/*	Driver for the ADXL345 accelerometer using SPI for communication
 *	(c)2019 by Marcel Meyer Garcia
 *	see LICENSE.txt
*/

/* NOTE: add "-Wl,-u,vfprintf -lprintf_flt -lm" under Linker->Miscellaneous->Other Linker Flags
	to enable float support for printf() or else it will print a question mark instead of the float number*/

#include "ADXL345.h"
#include "soft_uart.h"

int main(void)
{
	soft_uart_setup();
	
	if( !ADXL345_init() ){
		printf("init success\n");
	}else{
		printf("init error\n");
		return 1;
	}
    ADXL345_enterMeasurementMode();
	
    float x, y, z, roll, pitch;
	
	while (1){
		ADXL345_getAccelerationSI(&x, &y, &z); // get accelerations in m/s²
		ADXL345_getTiltAnglesDegrees(&roll, &pitch); // get roll and pitch inclination in °
		printf("x= %2.3f y= %2.3f z= %2.3f roll= %2.3f pitch= %2.3f\n", x, y, z, roll, pitch);
    }
}

