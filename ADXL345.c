/*	Driver for the ADXL345 accelerometer using SPI for communication
 *	(c)2019 by Marcel Meyer Garcia
 *	see LICENSE.txt
*/
#include "ADXL345.h"

/* INITIALIZE DEFAULT VALUES */
ADXL345_range ADXL345_currentRange = ADXL345_range_2g;
bool ADXL345_currentFullRes = false;

bool ADXL345_init(){
	struct SPI_config ADXL345_SPIcfg;
	ADXL345_SPIcfg.mode = SPI_MODE_3_gc; 
	ADXL345_SPIcfg.prescaler = SPI_PRESC_DIV64_gc;
	ADXL345_SPIcfg.useDoubleSpeed = true;
	ADXL345_SPIcfg.dataOrder = SPI_MSB_FIRST;
	SPI_init(&ADXL345_SPIcfg);
	// check device ID
	if( ADXL345_readRegister(0x00) != 0xE5 ) return true; // error: device ID doesn't match
	return false; // success
}

// read the content of a register of the ADXL345
uint8_t ADXL345_readRegister(uint8_t regAddr){
	SPI_CS_low();
	regAddr |= (1<<7); // append read bit
	SPI_transmit(regAddr); // send register address
	uint8_t regVal = SPI_transmit(0xFF);	// read register content
	SPI_CS_high();
	return regVal;
}
// check a single bit inside a register of the ADXL345
bool ADXL345_readRegisterBit(uint8_t regAddr, uint8_t bitPos){
	return ( (1<<bitPos) & ADXL345_readRegister(regAddr) );
}

// write to a register of the ADXL345
void ADXL345_writeRegister(uint8_t regAddr, uint8_t regVal){
	SPI_CS_low();
	SPI_transmit(regAddr); // send 7bit register address
	SPI_transmit(regVal);	// send register content
	SPI_CS_high();
}
// set or clear a single bit in a register (true = set bit, false = clear bit)
void ADXL345_writeRegisterBit(uint8_t regAddr, uint8_t bitPos, bool bitVal){
	uint8_t regVal = ADXL345_readRegister(regAddr);
	_delay_loop_1(3); // according to data sheet, CS must be high during at least 150ns between transmissions
					  // with a maximum CPU speed of 20Mhz this delay amounts to 150ns, so it is always >=150ns for all F_CPU
	regVal &=~ (1<<bitPos); // clear bit
	regVal |= (bitVal<<bitPos); // OR bit with value (i.e. either set or clear it depending on bitVal)
}

void ADXL345_enterStandbyMode(){
	ADXL345_writeRegisterBit(0x2D, 3, 0); // clear Measure-bit in POWER_CTL register (0x2D)
}

void ADXL345_enterMeasurementMode(){
ADXL345_writeRegisterBit(0x2D, 3, 1); // set Measure-bit in POWER_CTL register (0x2D)
}

// configure the sleep mode sampling frequency and enter sleep mode
void ADXL345_enterSleepMode(ADXL345_sleepModeFrequency freq){
	uint8_t regVal = ADXL345_readRegister(0x2D); // 0x2D = POWER_CTL register
	_delay_loop_1(3); // according to ADXL345 data sheet, CS must be high during at least 150ns between transmissions
						// with a maximum CPU speed of 20Mhz this delay amounts to 150ns, so it is always >=150ns for all F_CPU
	regVal |= (1<<2); // set Sleep-bit
	// set wakeup frequency bits
	regVal &=~ ((1<<1)|(1<<0));
	switch(freq){
		case ADXL345_sleepMode_1Hz:
			regVal |= ((1<<1)|(1<<0));
			break;
		case ADXL345_sleepMode_2Hz:
			regVal |= (1<<1);
			break;
		case ADXL345_sleepMode_4Hz:
			regVal |= (1<<0);
			break;
		case ADXL345_sleepMode_8Hz:
			break;
		default:
			break;	
	}
	ADXL345_writeRegister(0x2D, regVal); // write back modified register value
}

void ADXL345_wakeupFromSleepMode(){
	ADXL345_writeRegisterBit(0x2D, 2, 0); // clear Sleep-bit in POWER_CTL register (0x2D)
}
// link true = serially link activity and inactivity functions (assuming that both are activated)
// link false = act. and inact. functions work concurrently (see page 25 of data sheet)
void ADXL345_linkActivityInactivity(bool link){
	ADXL345_writeRegisterBit(0x2D, 5, link); // modify Link-bit in POWER_CTL register (0x2D)
}

void ADXL345_setAutoSleep(bool autosleep){
	ADXL345_writeRegisterBit(0x2D, 4, autosleep); // modify AUTO_SLEEP-bit in POWER_CTL register (0x2D)
}
// true = enable interrupt, false = disable interrupt
void ADXL345_setInterrupt(ADXL345_interrupt interrupt, bool interruptVal){
	ADXL345_writeRegisterBit(0x2E, interrupt, interruptVal);
}

//void ADXL345_enableInterrupt(ADXL345_interrupt interrupt){
	//uint8_t regVal = ADXL345_readRegister(0x2E); // read INT_ENABLE register
	//_delay_loop_1(3); // according to ADXL345 data sheet, CS must be high during at least 150ns between transmissions
					  //// with a maximum CPU speed of 20Mhz this delay amounts to 150ns, so it is always >=150ns for all F_CPU
	//switch (interrupt){
		//case ADXL345_dataReadyInterrupt:
			//regVal |= (1<<7);
			//break;
		//case ADXL345_singleTapInterrupt:
			//regVal |= (1<<6);
			//break;
		//case ADXL345_doubleTapInterrupt:
			//regVal |= (1<<5);
			//break;
		//case ADXL345_activityInterrupt:
			//regVal |= (1<<4);
			//break;
		//case ADXL345_inactivityInterrupt:
			//regVal |= (1<<3);
			//break;
		//case ADXL345_freeFallInterrupt:
			//regVal |= (1<<2);
			//break;
		//case ADXL345_watermarkInterrupt:
			//regVal |= (1<<1);
			//break;
		//case ADXL345_overrunInterrupt:
			//regVal |= (1<<0);
		//break;
		//default:
		//break;
	//}
	//ADXL345_writeRegister(0x2E, regVal); // write back modified register value
//}
//
//void ADXL345_disableInterrupt(ADXL345_interrupt interrupt){
	//uint8_t regVal = ADXL345_readRegister(0x2E); // read INT_ENABLE register
	//_delay_loop_1(3); // according to ADXL345 data sheet, CS must be high during at least 150ns between transmissions
					  //// with a maximum CPU speed of 20Mhz this delay amounts to 150ns, so it is always >=150ns for all F_CPU
	//switch (interrupt){
		//case ADXL345_dataReadyInterrupt:
		//regVal &=~(1<<7);
		//break;
		//case ADXL345_singleTapInterrupt:
		//regVal &=~(1<<6);
		//break;
		//case ADXL345_doubleTapInterrupt:
		//regVal &=~(1<<5);
		//break;
		//case ADXL345_activityInterrupt:
		//regVal &=~(1<<4);
		//break;
		//case ADXL345_inactivityInterrupt:
		//regVal &=~(1<<3);
		//break;
		//case ADXL345_freeFallInterrupt:
		//regVal &=~(1<<2);
		//break;
		//case ADXL345_watermarkInterrupt:
		//regVal &=~(1<<1);
		//break;
		//case ADXL345_overrunInterrupt:
		//regVal &=~(1<<0);
		//break;
		//default:
		//break;
	//}
	//ADXL345_writeRegister(0x2E, regVal); // write back modified register value
//}

// check a certain interrupt flag, argument: interrupt source to be checked; return value: true if the specified interrupt has been triggered, false if not
bool ADXL345_checkInterrupt(ADXL345_interrupt interrupt){
	return ADXL345_readRegisterBit(0x30, interrupt);
}

// map the given interrupt to the INT1 pin (the interrupt still has to be enabled for it to work)
void ADXL345_mapInterrupt_1(ADXL345_interrupt interrupt){
	ADXL345_writeRegisterBit(0x2F, interrupt, 0);
}

// map the given interrupt to the INT2 pin (the interrupt still has to be enabled for it to work)
void ADXL345_mapInterrupt_2(ADXL345_interrupt interrupt){
	ADXL345_writeRegisterBit(0x2F, interrupt, 1);
}

void ADXL345_setInterruptPolarity(ADXL345_interruptPolarity pol){
	ADXL345_writeRegisterBit(0x31, 5, pol);
}
// enable (true) or disable (false) full resolution mode (see page 27 of the data sheet)
void ADXL345_setFullResolution(bool fullRes){
	ADXL345_writeRegisterBit(0x31, 3, fullRes);
	ADXL345_currentFullRes = fullRes;
}

void ADXL345_setRange(ADXL345_range range){
	uint8_t regVal = ADXL345_readRegister(0x31); // read DATA_FORMAT register
	_delay_loop_1(3); // according to ADXL345 data sheet, CS must be high during at least 150ns between transmissions
					  // with a maximum CPU speed of 20Mhz this delay amounts to 150ns, so it is always >=150ns for all F_CPU
	regVal &=~ ((1<<1) | (1<<0)); // clear both Range-bits
	regVal |= (uint8_t) range;
	ADXL345_writeRegister(0x31, regVal); // write back modified register value
	ADXL345_currentRange = range;
}

void ADXL_configFIFO(ADXL345_FIFOmode mode){
	uint8_t regVal = ADXL345_readRegister(0x38); // read FIFO_CTL register
	_delay_loop_1(3); // according to ADXL345 data sheet, CS must be high during at least 150ns between transmissions
					  // with a maximum CPU speed of 20Mhz this delay amounts to 150ns, so it is always >=150ns for all F_CPU
	regVal &=~ ((1<<7) | (1<<6)); // clear both FIFO_MODE-bits
	regVal |= (((uint8_t)mode)<<6); // config both FIFO_MODE-bits
	ADXL345_writeRegister(0x38, regVal); // write back modified register value
}

// returns amount of samples available in the ADXL345's FIFO buffer
uint8_t ADXL345_getFIFOSamplesAvailable(){
	return (0x3F & ADXL345_readRegister(0x39)); // read FIFO_CTL register, return 6 lower bits
}

// read acceleration data of all three axes in one multiple-byte (a.k.a. burst-read) transmission
void ADXL345_getRawData(int16_t* x, int16_t* y, int16_t* z){
	SPI_CS_low();
	// read all data registers in one burst
	SPI_transmit( 0x32 | (1<<7) | (1<<6) ); // DATAX0 address + read-bit + multiple-bit
	*x = SPI_transmit(0xFF); // read x-data LSB
	*x |= (SPI_transmit(0xFF)<<8); // read x-data MSB
	*y = SPI_transmit(0xFF); // read y-data LSB
	*y |= (SPI_transmit(0xFF)<<8); // read y-data MSB
	*z = SPI_transmit(0xFF); // read z-data LSB
	*z |= (SPI_transmit(0xFF)<<8); // read z-data MSB
	SPI_CS_high();
}

// get acceleration data as float in SI unit (m/s^2)
void ADXL345_getAccelerationSI(float* x_f, float* y_f, float* z_f){
	int16_t x, y, z;
	ADXL345_getRawData(&x, &y, &z);
	// if Full Resolution is enabled, the scale factor is always 4mg/LSB = 0.03924 (m/s^2)/LSB regardless of the range setting
	if(ADXL345_currentFullRes){
		*x_f = x * 0.004;
		*y_f = y * 0.004;
		*z_f = z * 0.004;
	// if Full Resolution is disabled, the scale factor depends on the range setting and is calculated as follows:
	// (max_g - min_g) / 2^(10bit) * 9.81 (m/s^2)/g
	// e.g. with the +/-2g setting:
	// (+2g - (-2g)) / 2^(10bit) * 9.81 (m/s^2)/g = 0.0383 (m/s^2)/bit
	}else{
		float scale_factor;
		switch (ADXL345_currentRange){
			case ADXL345_range_2g:
				scale_factor = 0.03830; 
				break;
			case ADXL345_range_4g:
				scale_factor = 0.07664;
				break;
			case ADXL345_range_8g:
				scale_factor = 0.15328;
				break;
			case ADXL345_range_16g:
				scale_factor = 0.30656;
				break;
			default:
				break;
		}
		*x_f = x * scale_factor;
		*y_f = y * scale_factor;
		*z_f = z * scale_factor;
	}
}
// read acceleration and calculate the tilt (roll and yaw), NOTE: works only when the sensor is static (no motion)
void ADXL345_getTiltAnglesDegrees(float* roll, float* pitch){
	float x, y, z;
	ADXL345_getAccelerationSI(&x, &y, &z);
	z *= z; // since we will need z^2 twice, computing it here makes the calculation more efficient
	*roll = 180.0 / M_PI * atan2( y, sqrt( x*x + z ) ); // compute roll angle and convert from radians to degrees
	*pitch = 180.0 / M_PI * atan2( x, sqrt( y*y + z ); // compute roll angle and convert from radians to degrees
}