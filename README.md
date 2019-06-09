# ADXL345_lib
library for the ADXL345 accelerometer communicating via 4-wire SPI
written for the AVR ATtiny3216 (ATtiny-1 series) as part of the *BLEthingy*-project

when porting to another platform, the following changes have to be made:
* setup SPI in mode 3 inside *ADXL345_init()*
* provide *SPI_CS_low()* and *SPI_CS_high()* which pull the CS line high or low
* provide an *uint8_t SPI_transmit(uint8_t)* which transmits and receives one byte
* if needed (depends on your CPU speed), provide a *_delay_loop_1(3);* which delays for approx. 150ns
