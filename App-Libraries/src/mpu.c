/*
 * mpu.c
 *
 *  Created on: Aug 31, 2020
 *      Author: aninda
 */
#include "mpu.h"

static int8_t MPU_readRegisters(uint8_t subAddress, uint8_t count, uint8_t *data);
static int8_t MPU_writeRegister(uint8_t subAddress, uint8_t data);
static int8_t MPU_writeAK8963Register(uint8_t subAddress, uint8_t data);
static int8_t MPU_readAK8963Registers(uint8_t subAddress, uint8_t count, uint8_t *dest);

#define bool uint8_t

// track success of interacting with sensor
static int8_t _status;
// buffer for reading from sensor
static uint8_t _buffer[21];
// data counts
static int16_t _axcounts,_aycounts,_azcounts;
static int16_t _gxcounts,_gycounts,_gzcounts;
static int16_t _hxcounts,_hycounts,_hzcounts;
static int16_t _tcounts;
// data buffer
//static float _ax, _ay, _az;
//static float _gx, _gy, _gz;
//static float _hx, _hy, _hz;
//static float _t;
// wake on motion
static uint8_t _womThreshold;
// scale factors
static float _accelScale;
static float _gyroScale;
static float _magScaleX, _magScaleY, _magScaleZ;
static const float _tempScale = 333.87f;
static const float _tempOffset = 21.0f;
// configuration
static AccelRange _accelRange;
static GyroRange _gyroRange;
static DlpfBandwidth _bandwidth;
static uint8_t _srd;
// gyro bias estimation
static size_t _numSamples = 100;
static double _gxbD, _gybD, _gzbD;
static float _gxb, _gyb, _gzb;
// accel bias and scale factor estimation
static double _axbD, _aybD, _azbD;
static float _axmax, _aymax, _azmax;
static float _axmin, _aymin, _azmin;
static float _axb, _ayb, _azb;
static float _axs = 1.0f;
static float _ays = 1.0f;
static float _azs = 1.0f;
// magnetometer bias and scale factor estimation
static uint16_t _maxCounts = 1000;
static float _deltaThresh = 0.3f;
static uint8_t _coeff = 8;
static uint16_t _counter;
static float _framedelta, _delta;
static float _hxfilt, _hyfilt, _hzfilt;
static float _hxmax, _hymax, _hzmax;
static float _hxmin, _hymin, _hzmin;
static float _hxb, _hyb, _hzb;
static float _hxs = 1.0f;
static float _hys = 1.0f;
static float _hzs = 1.0f;
static float _avgs;
// transformation matrix
/* transform the accel and gyro axes to match the magnetometer axes */
static const int16_t tX[3] = {0,  1,  0};
static const int16_t tY[3] = {1,  0,  0};
static const int16_t tZ[3] = {0,  0, -1};
// constants
static const float G = 9.807f;
static const float _d2r = 3.14159265359f/180.0f;


int8_t MPU_Init(void)
{
	// Initialize the IIC peripheral
	IIC_Init();

	HAL_Delay(1);

	// reset the MPU9250
	MPU_writeRegister(PWR_MGMNT_1, PWR_RESET);

	HAL_Delay(1);

	// select clock source to gyro
	if (MPU_writeRegister(PWR_MGMNT_1, CLOCK_SEL_PLL) < 0)
	{
		return -1;
	}
	// enable I2C master mode
	if (MPU_writeRegister(USER_CTRL, I2C_MST_EN) < 0)
	{
		return -2;
	}
	// set the I2C bus speed to 400 kHz
	if (MPU_writeRegister(I2C_MST_CTRL, I2C_MST_CLK) < 0)
	{
		return -3;
	}
	// set AK8963 to Power Down
	MPU_writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN);
	// reset the MPU9250
	MPU_writeRegister(PWR_MGMNT_1, PWR_RESET);
	// wait for MPU-9250 to come back up
	HAL_Delay(1);
	// reset the AK8963
	MPU_writeAK8963Register(AK8963_CNTL2, AK8963_RESET);
	// select clock source to gyro
	if (MPU_writeRegister(PWR_MGMNT_1, CLOCK_SEL_PLL) < 0)
	{
		return -4;
	}
	// check the WHO AM I byte, expected value is 0x71 (decimal 113) or 0x73 (decimal 115)
	if ((MPU_whoAmI() != 113) && (MPU_whoAmI() != 115))
	{
		return -5;
	}
	// enable accelerometer and gyro
	if (MPU_writeRegister(PWR_MGMNT_2, SEN_ENABLE) < 0)
	{
		return -6;
	}
	// setting accel range to 16G as default
	if (MPU_writeRegister(ACCEL_CONFIG, ACCEL_FS_SEL_16G) < 0)
	{
		return -7;
	}
	_accelScale = G * 16.0f / 32767.5f; // setting the accel scale to 16G
	_accelRange = ACCEL_RANGE_16G;
	// setting the gyro range to 2000DPS as default
	if (MPU_writeRegister(GYRO_CONFIG, GYRO_FS_SEL_2000DPS) < 0)
	{
		return -8;
	}
	_gyroScale = 2000.0f / 32767.5f * _d2r; // setting the gyro scale to 2000DPS
	_gyroRange = GYRO_RANGE_2000DPS;
	// setting bandwidth to 184Hz as default
	if (MPU_writeRegister(ACCEL_CONFIG2, ACCEL_DLPF_184) < 0)
	{
		return -9;
	}
	if (MPU_writeRegister(CONFIG, GYRO_DLPF_184) < 0)
	{ // setting gyro bandwidth to 184Hz
		return -10;
	}
	_bandwidth = DLPF_BANDWIDTH_184HZ;
	// setting the sample rate divider to 0 as default
	if (MPU_writeRegister(SMPDIV, 0x00) < 0)
	{
		return -11;
	}
	_srd = 0;
	// enable I2C master mode
	if (MPU_writeRegister(USER_CTRL, I2C_MST_EN) < 0)
	{
		return -12;
	}
	// set the I2C bus speed to 400 kHz
	if (MPU_writeRegister(I2C_MST_CTRL, I2C_MST_CLK) < 0)
	{
		return -13;
	}
	// check AK8963 WHO AM I register, expected value is 0x48 (decimal 72)
//	if (MPU_whoAmIAK8963() != 72)
//	{
//		return -14;
//	}
	/* get the magnetometer calibration */
	// set AK8963 to Power Down
	if (MPU_writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN) < 0)
	{
		return -15;
	}
	HAL_Delay(100); // long wait between AK8963 mode changes
	// set AK8963 to FUSE ROM access
	if (MPU_writeAK8963Register(AK8963_CNTL1, AK8963_FUSE_ROM) < 0)
	{
		return -16;
	}
	HAL_Delay(100); // long wait between AK8963 mode changes
	// read the AK8963 ASA registers and compute magnetometer scale factors
	MPU_readAK8963Registers(AK8963_ASA, 3, _buffer);
	_magScaleX = ((((float) _buffer[0]) - 128.0f) / (256.0f) + 1.0f) * 4912.0f / 32760.0f; // micro Tesla
	_magScaleY = ((((float) _buffer[1]) - 128.0f) / (256.0f) + 1.0f) * 4912.0f / 32760.0f; // micro Tesla
	_magScaleZ = ((((float) _buffer[2]) - 128.0f) / (256.0f) + 1.0f) * 4912.0f / 32760.0f; // micro Tesla
	// set AK8963 to Power Down
	if (MPU_writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN) < 0)
	{
		return -17;
	}
	HAL_Delay(100); // long wait between AK8963 mode changes
	// set AK8963 to 16 bit resolution, 100 Hz update rate
	if (MPU_writeAK8963Register(AK8963_CNTL1, AK8963_CNT_MEAS2) < 0)
	{
		return -18;
	}
	HAL_Delay(100); // long wait between AK8963 mode changes
	// select clock source to gyro
	if (MPU_writeRegister(PWR_MGMNT_1, CLOCK_SEL_PLL) < 0)
	{
		return -19;
	}
	// instruct the MPU9250 to get 7 bytes of data from the AK8963 at the sample rate
	MPU_readAK8963Registers(AK8963_HXL, 7, _buffer);
	// estimate gyro bias
	if (calibrateGyro() < 0)
	{
		return -20;
	}
	// successful init, return 1
	return 1;
}

/* gets the MPU9250 WHO_AM_I register value, expected to be 0x71 */
int8_t MPU_whoAmI()
{
	// read the WHO AM I register
	if (MPU_readRegisters(WHO_AM_I, 1, _buffer) < 0)
	{
		return -1;
	}
	// return the register value
	return _buffer[0];
}

/* gets the AK8963 WHO_AM_I register value, expected to be 0x48 */
int8_t MPU_whoAmIAK8963()
{
	// read the WHO AM I register
	if (MPU_readAK8963Registers(AK8963_WHO_AM_I, 1, _buffer) < 0)
	{
		return -1;
	}
	// return the register value
	return _buffer[0];
}

int8_t MPU_ReadRegister(uint8_t registeraddr){
	MPU_readRegisters(registeraddr, 1, _buffer);
	return _buffer[0];
}

void MPU_WriteRegister(uint8_t registeraddr, uint8_t data){
	MPU_writeRegister(registeraddr, data);
}


/* sets the accelerometer full scale range to values other than default */
int8_t setAccelRange(AccelRange range)
{

	switch (range)
	{
		case ACCEL_RANGE_2G:
		{
			// setting the accel range to 2G
			if (MPU_writeRegister(ACCEL_CONFIG, ACCEL_FS_SEL_2G) < 0)
			{
				return -1;
			}
			_accelScale = G * 2.0f / 32767.5f; // setting the accel scale to 2G
			break;
		}
		case ACCEL_RANGE_4G:
		{
			// setting the accel range to 4G
			if (MPU_writeRegister(ACCEL_CONFIG, ACCEL_FS_SEL_4G) < 0)
			{
				return -1;
			}
			_accelScale = G * 4.0f / 32767.5f; // setting the accel scale to 4G
			break;
		}
		case ACCEL_RANGE_8G:
		{
			// setting the accel range to 8G
			if (MPU_writeRegister(ACCEL_CONFIG, ACCEL_FS_SEL_8G) < 0)
			{
				return -1;
			}
			_accelScale = G * 8.0f / 32767.5f; // setting the accel scale to 8G
			break;
		}
		case ACCEL_RANGE_16G:
		{
			// setting the accel range to 16G
			if (MPU_writeRegister(ACCEL_CONFIG, ACCEL_FS_SEL_16G) < 0)
			{
				return -1;
			}
			_accelScale = G * 16.0f / 32767.5f; // setting the accel scale to 16G
			break;
		}
	}
	_accelRange = range;
	return 1;
}

/* sets the gyro full scale range to values other than default */
int8_t setGyroRange(GyroRange range)
{

	switch (range)
	{
		case GYRO_RANGE_250DPS:
		{
			// setting the gyro range to 250DPS
			if (MPU_writeRegister(GYRO_CONFIG, GYRO_FS_SEL_250DPS) < 0)
			{
				return -1;
			}
			_gyroScale = 250.0f / 32767.5f * _d2r; // setting the gyro scale to 250DPS
			break;
		}
		case GYRO_RANGE_500DPS:
		{
			// setting the gyro range to 500DPS
			if (MPU_writeRegister(GYRO_CONFIG, GYRO_FS_SEL_500DPS) < 0)
			{
				return -1;
			}
			_gyroScale = 500.0f / 32767.5f * _d2r; // setting the gyro scale to 500DPS
			break;
		}
		case GYRO_RANGE_1000DPS:
		{
			// setting the gyro range to 1000DPS
			if (MPU_writeRegister(GYRO_CONFIG, GYRO_FS_SEL_1000DPS) < 0)
			{
				return -1;
			}
			_gyroScale = 1000.0f / 32767.5f * _d2r; // setting the gyro scale to 1000DPS
			break;
		}
		case GYRO_RANGE_2000DPS:
		{
			// setting the gyro range to 2000DPS
			if (MPU_writeRegister(GYRO_CONFIG, GYRO_FS_SEL_2000DPS) < 0)
			{
				return -1;
			}
			_gyroScale = 2000.0f / 32767.5f * _d2r; // setting the gyro scale to 2000DPS
			break;
		}
	}
	_gyroRange = range;
	return 1;
}

/* sets the DLPF bandwidth to values other than default */
int8_t setDlpfBandwidth(DlpfBandwidth bandwidth)
{
	switch (bandwidth)
	{
		case DLPF_BANDWIDTH_184HZ:
		{
			if (MPU_writeRegister(ACCEL_CONFIG2, ACCEL_DLPF_184) < 0)
			{ // setting accel bandwidth to 184Hz
				return -1;
			}
			if (MPU_writeRegister(CONFIG, GYRO_DLPF_184) < 0)
			{ // setting gyro bandwidth to 184Hz
				return -2;
			}
			break;
		}
		case DLPF_BANDWIDTH_92HZ:
		{
			if (MPU_writeRegister(ACCEL_CONFIG2, ACCEL_DLPF_92) < 0)
			{ // setting accel bandwidth to 92Hz
				return -1;
			}
			if (MPU_writeRegister(CONFIG, GYRO_DLPF_92) < 0)
			{ // setting gyro bandwidth to 92Hz
				return -2;
			}
			break;
		}
		case DLPF_BANDWIDTH_41HZ:
		{
			if (MPU_writeRegister(ACCEL_CONFIG2, ACCEL_DLPF_41) < 0)
			{ // setting accel bandwidth to 41Hz
				return -1;
			}
			if (MPU_writeRegister(CONFIG, GYRO_DLPF_41) < 0)
			{ // setting gyro bandwidth to 41Hz
				return -2;
			}
			break;
		}
		case DLPF_BANDWIDTH_20HZ:
		{
			if (MPU_writeRegister(ACCEL_CONFIG2, ACCEL_DLPF_20) < 0)
			{ // setting accel bandwidth to 20Hz
				return -1;
			}
			if (MPU_writeRegister(CONFIG, GYRO_DLPF_20) < 0)
			{ // setting gyro bandwidth to 20Hz
				return -2;
			}
			break;
		}
		case DLPF_BANDWIDTH_10HZ:
		{
			if (MPU_writeRegister(ACCEL_CONFIG2, ACCEL_DLPF_10) < 0)
			{ // setting accel bandwidth to 10Hz
				return -1;
			}
			if (MPU_writeRegister(CONFIG, GYRO_DLPF_10) < 0)
			{ // setting gyro bandwidth to 10Hz
				return -2;
			}
			break;
		}
		case DLPF_BANDWIDTH_5HZ:
		{
			if (MPU_writeRegister(ACCEL_CONFIG2, ACCEL_DLPF_5) < 0)
			{ // setting accel bandwidth to 5Hz
				return -1;
			}
			if (MPU_writeRegister(CONFIG, GYRO_DLPF_5) < 0)
			{ // setting gyro bandwidth to 5Hz
				return -2;
			}
			break;
		}
	}
	_bandwidth = bandwidth;
	return 1;
}

/* sets the sample rate divider to values other than default */
int8_t setSrd(uint8_t srd)
{
	/* setting the sample rate divider to 19 to facilitate setting up magnetometer */
	if (MPU_writeRegister(SMPDIV, 19) < 0)
	{ // setting the sample rate divider
		return -1;
	}
	if (srd > 9)
	{
		// set AK8963 to Power Down
		if (MPU_writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN) < 0)
		{
			return -2;
		}
		HAL_Delay(100); // long wait between AK8963 mode changes
		// set AK8963 to 16 bit resolution, 8 Hz update rate
		if (MPU_writeAK8963Register(AK8963_CNTL1, AK8963_CNT_MEAS1) < 0)
		{
			return -3;
		}
		HAL_Delay(100); // long wait between AK8963 mode changes
		// instruct the MPU9250 to get 7 bytes of data from the AK8963 at the sample rate
		MPU_readAK8963Registers(AK8963_HXL, 7, _buffer);
	}
	else
	{
		// set AK8963 to Power Down
		if (MPU_writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN) < 0)
		{
			return -2;
		}
		HAL_Delay(100); // long wait between AK8963 mode changes
		// set AK8963 to 16 bit resolution, 100 Hz update rate
		if (MPU_writeAK8963Register(AK8963_CNTL1, AK8963_CNT_MEAS2) < 0)
		{
			return -3;
		}
		HAL_Delay(100); // long wait between AK8963 mode changes
		// instruct the MPU9250 to get 7 bytes of data from the AK8963 at the sample rate
		MPU_readAK8963Registers(AK8963_HXL, 7, _buffer);
	}
	/* setting the sample rate divider */
	if (MPU_writeRegister(SMPDIV, srd) < 0)
	{ // setting the sample rate divider
		return -4;
	}
	_srd = srd;
	return 1;
}

/* enables the data ready interrupt */
int8_t enableDataReadyInterrupt()
{
	/* setting the interrupt */
	if (MPU_writeRegister(INT_PIN_CFG, INT_PULSE_50US) < 0)
	{ // setup interrupt, 50 us pulse
		return -1;
	}
	if (MPU_writeRegister(INT_ENABLE, INT_RAW_RDY_EN) < 0)
	{ // set to data ready
		return -2;
	}
	return 1;
}

/* disables the data ready interrupt */
int8_t disableDataReadyInterrupt()
{
	if (MPU_writeRegister(INT_ENABLE, INT_DISABLE) < 0)
	{ // disable interrupt
		return -1;
	}
	return 1;
}

/* configures and enables wake on motion, low power mode */
int8_t enableWakeOnMotion(float womThresh_mg, LpAccelOdr odr)
{
	// set AK8963 to Power Down
	MPU_writeAK8963Register(AK8963_CNTL1, AK8963_PWR_DOWN);
	// reset the MPU9250
	MPU_writeRegister(PWR_MGMNT_1, PWR_RESET);
	// wait for MPU-9250 to come back up
	HAL_Delay(1);
	if (MPU_writeRegister(PWR_MGMNT_1, 0x00) < 0)
	{ // cycle 0, sleep 0, standby 0
		return -1;
	}
	if (MPU_writeRegister(PWR_MGMNT_2, DIS_GYRO) < 0)
	{ // disable gyro measurements
		return -2;
	}
	if (MPU_writeRegister(ACCEL_CONFIG2, ACCEL_DLPF_184) < 0)
	{ // setting accel bandwidth to 184Hz
		return -3;
	}
	if (MPU_writeRegister(INT_ENABLE, INT_WOM_EN) < 0)
	{ // enabling interrupt to wake on motion
		return -4;
	}
	if (MPU_writeRegister(MOT_DETECT_CTRL, (ACCEL_INTEL_EN | ACCEL_INTEL_MODE)) < 0)
	{ // enabling accel hardware intelligence
		return -5;
	}
//	_womThreshold = map(womThresh_mg, 0, 1020, 0, 255);
	if (MPU_writeRegister(WOM_THR, _womThreshold) < 0)
	{ // setting wake on motion threshold
		return -6;
	}
	if (MPU_writeRegister(LP_ACCEL_ODR, (uint8_t) odr) < 0)
	{ // set frequency of wakeup
		return -7;
	}
	if (MPU_writeRegister(PWR_MGMNT_1, PWR_CYCLE) < 0)
	{ // switch to accel low power mode
		return -8;
	}
	return 1;
}

/* reads the most current data from MPU9250 and stores in buffer */
int8_t MPU_readSensor()
{

	// grab the data from the MPU9250
	if (MPU_readRegisters(ACCEL_OUT, 21, _buffer) < 0)
	{
		return -1;
	}
	// combine into 16 bit values
	_axcounts = (((int16_t) _buffer[0]) << 8) | _buffer[1];
	_aycounts = (((int16_t) _buffer[2]) << 8) | _buffer[3];
	_azcounts = (((int16_t) _buffer[4]) << 8) | _buffer[5];
	_tcounts = (((int16_t) _buffer[6]) << 8) | _buffer[7];
	_gxcounts = (((int16_t) _buffer[8]) << 8) | _buffer[9];
	_gycounts = (((int16_t) _buffer[10]) << 8) | _buffer[11];
	_gzcounts = (((int16_t) _buffer[12]) << 8) | _buffer[13];
	_hxcounts = (((int16_t) _buffer[15]) << 8) | _buffer[14];
	_hycounts = (((int16_t) _buffer[17]) << 8) | _buffer[16];
	_hzcounts = (((int16_t) _buffer[19]) << 8) | _buffer[18];
	// transform and convert to float values
	_accelerometer_val[0] = (((float) (tX[0] * _axcounts + tX[1] * _aycounts + tX[2] * _azcounts) * _accelScale) - _axb) * _axs;
	_accelerometer_val[1] = (((float) (tY[0] * _axcounts + tY[1] * _aycounts + tY[2] * _azcounts) * _accelScale) - _ayb) * _ays;
	_accelerometer_val[2] = (((float) (tZ[0] * _axcounts + tZ[1] * _aycounts + tZ[2] * _azcounts) * _accelScale) - _azb) * _azs;
	_gyroscope_val[0] = ((float) (tX[0] * _gxcounts + tX[1] * _gycounts + tX[2] * _gzcounts) * _gyroScale) - _gxb;
	_gyroscope_val[1] = ((float) (tY[0] * _gxcounts + tY[1] * _gycounts + tY[2] * _gzcounts) * _gyroScale) - _gyb;
	_gyroscope_val[2] = ((float) (tZ[0] * _gxcounts + tZ[1] * _gycounts + tZ[2] * _gzcounts) * _gyroScale) - _gzb;
	_magnetometer_val[0] = (((float) (_hxcounts) * _magScaleX) - _hxb) * _hxs;
	_magnetometer_val[1] = (((float) (_hycounts) * _magScaleY) - _hyb) * _hys;
	_magnetometer_val[2] = (((float) (_hzcounts) * _magScaleZ) - _hzb) * _hzs;
	_temperature_val = ((((float) _tcounts) - _tempOffset) / _tempScale) + _tempOffset;
	return 1;
}

/* returns the accelerometer measurement in the x direction, m/s/s */
float getAccelX_mss()
{
	return _accelerometer_val[0];
}

/* returns the accelerometer measurement in the y direction, m/s/s */
float getAccelY_mss()
{
	return _accelerometer_val[1];
}

/* returns the accelerometer measurement in the z direction, m/s/s */
float getAccelZ_mss()
{
	return _accelerometer_val[2];
}

/* returns the gyroscope measurement in the x direction, rad/s */
float getGyroX_rads()
{
	return _gyroscope_val[0];
}

/* returns the gyroscope measurement in the y direction, rad/s */
float getGyroY_rads()
{
	return _gyroscope_val[1];
}

/* returns the gyroscope measurement in the z direction, rad/s */
float getGyroZ_rads()
{
	return _gyroscope_val[2];
}

/* returns the magnetometer measurement in the x direction, uT */
float getMagX_uT()
{
	return _magnetometer_val[0];
}

/* returns the magnetometer measurement in the y direction, uT */
float getMagY_uT()
{
	return _magnetometer_val[1];
}

/* returns the magnetometer measurement in the z direction, uT */
float getMagZ_uT()
{
	return _magnetometer_val[2];
}

/* returns the die temperature, C */
float getTemperature_C()
{
	return _temperature_val;
}

/* estimates the gyro biases */
int8_t calibrateGyro()
{
	// set the range, bandwidth, and srd
	if (setGyroRange(GYRO_RANGE_250DPS) < 0)
	{
		return -1;
	}
	if (setDlpfBandwidth(DLPF_BANDWIDTH_20HZ) < 0)
	{
		return -2;
	}
	if (setSrd(19) < 0)
	{
		return -3;
	}

	// take samples and find bias
	_gxbD = 0;
	_gybD = 0;
	_gzbD = 0;
	for (size_t i = 0; i < _numSamples; i++)
	{
		MPU_readSensor();
		_gxbD += (getGyroX_rads() + _gxb) / ((double) _numSamples);
		_gybD += (getGyroY_rads() + _gyb) / ((double) _numSamples);
		_gzbD += (getGyroZ_rads() + _gzb) / ((double) _numSamples);
		HAL_Delay(20);
	}
	_gxb = (float) _gxbD;
	_gyb = (float) _gybD;
	_gzb = (float) _gzbD;

	// set the range, bandwidth, and srd back to what they were
	if (setGyroRange(_gyroRange) < 0)
	{
		return -4;
	}
	if (setDlpfBandwidth(_bandwidth) < 0)
	{
		return -5;
	}
	if (setSrd(_srd) < 0)
	{
		return -6;
	}
	return 1;
}

/* returns the gyro bias in the X direction, rad/s */
float getGyroBiasX_rads()
{
	return _gxb;
}

/* returns the gyro bias in the Y direction, rad/s */
float getGyroBiasY_rads()
{
	return _gyb;
}

/* returns the gyro bias in the Z direction, rad/s */
float getGyroBiasZ_rads()
{
	return _gzb;
}

/* sets the gyro bias in the X direction to bias, rad/s */
void setGyroBiasX_rads(float bias)
{
	_gxb = bias;
}

/* sets the gyro bias in the Y direction to bias, rad/s */
void setGyroBiasY_rads(float bias)
{
	_gyb = bias;
}

/* sets the gyro bias in the Z direction to bias, rad/s */
void setGyroBiasZ_rads(float bias)
{
	_gzb = bias;
}

/* finds bias and scale factor calibration for the accelerometer,
 this should be run for each axis in each direction (6 total) to find
 the min and max values along each */
int8_t calibrateAccel()
{
	// set the range, bandwidth, and srd
	if (setAccelRange(ACCEL_RANGE_2G) < 0)
	{
		return -1;
	}
	if (setDlpfBandwidth(DLPF_BANDWIDTH_20HZ) < 0)
	{
		return -2;
	}
	if (setSrd(19) < 0)
	{
		return -3;
	}

	// take samples and find min / max
	_axbD = 0;
	_aybD = 0;
	_azbD = 0;
	for (size_t i = 0; i < _numSamples; i++)
	{
		MPU_readSensor();
		_axbD += (getAccelX_mss() / _axs + _axb) / ((double) _numSamples);
		_aybD += (getAccelY_mss() / _ays + _ayb) / ((double) _numSamples);
		_azbD += (getAccelZ_mss() / _azs + _azb) / ((double) _numSamples);
		HAL_Delay(20);
	}
	if (_axbD > 9.0f)
	{
		_axmax = (float) _axbD;
	}
	if (_aybD > 9.0f)
	{
		_aymax = (float) _aybD;
	}
	if (_azbD > 9.0f)
	{
		_azmax = (float) _azbD;
	}
	if (_axbD < -9.0f)
	{
		_axmin = (float) _axbD;
	}
	if (_aybD < -9.0f)
	{
		_aymin = (float) _aybD;
	}
	if (_azbD < -9.0f)
	{
		_azmin = (float) _azbD;
	}

	// find bias and scale factor
	if ((abs(_axmin) > 9.0f) && (abs(_axmax) > 9.0f))
	{
		_axb = (_axmin + _axmax) / 2.0f;
		_axs = G / ((abs(_axmin) + abs(_axmax)) / 2.0f);
	}
	if ((abs(_aymin) > 9.0f) && (abs(_aymax) > 9.0f))
	{
		_ayb = (_aymin + _aymax) / 2.0f;
		_ays = G / ((abs(_aymin) + abs(_aymax)) / 2.0f);
	}
	if ((abs(_azmin) > 9.0f) && (abs(_azmax) > 9.0f))
	{
		_azb = (_azmin + _azmax) / 2.0f;
		_azs = G / ((abs(_azmin) + abs(_azmax)) / 2.0f);
	}

	// set the range, bandwidth, and srd back to what they were
	if (setAccelRange(_accelRange) < 0)
	{
		return -4;
	}
	if (setDlpfBandwidth(_bandwidth) < 0)
	{
		return -5;
	}
	if (setSrd(_srd) < 0)
	{
		return -6;
	}
	return 1;
}

/* returns the accelerometer bias in the X direction, m/s/s */
float getAccelBiasX_mss()
{
	return _axb;
}

/* returns the accelerometer scale factor in the X direction */
float getAccelScaleFactorX()
{
	return _axs;
}

/* returns the accelerometer bias in the Y direction, m/s/s */
float getAccelBiasY_mss()
{
	return _ayb;
}

/* returns the accelerometer scale factor in the Y direction */
float getAccelScaleFactorY()
{
	return _ays;
}

/* returns the accelerometer bias in the Z direction, m/s/s */
float getAccelBiasZ_mss()
{
	return _azb;
}

/* returns the accelerometer scale factor in the Z direction */
float getAccelScaleFactorZ()
{
	return _azs;
}

/* sets the accelerometer bias (m/s/s) and scale factor in the X direction */
void setAccelCalX(float bias, float scaleFactor)
{
	_axb = bias;
	_axs = scaleFactor;
}

/* sets the accelerometer bias (m/s/s) and scale factor in the Y direction */
void setAccelCalY(float bias, float scaleFactor)
{
	_ayb = bias;
	_ays = scaleFactor;
}

/* sets the accelerometer bias (m/s/s) and scale factor in the Z direction */
void setAccelCalZ(float bias, float scaleFactor)
{
	_azb = bias;
	_azs = scaleFactor;
}

/* finds bias and scale factor calibration for the magnetometer,
 the sensor should be rotated in a figure 8 motion until complete */
int8_t calibrateMag()
{
	// set the srd
	if (setSrd(19) < 0)
	{
		return -1;
	}

	// get a starting set of data
	MPU_readSensor();
	_hxmax = getMagX_uT();
	_hxmin = getMagX_uT();
	_hymax = getMagY_uT();
	_hymin = getMagY_uT();
	_hzmax = getMagZ_uT();
	_hzmin = getMagZ_uT();

	// collect data to find max / min in each channel
	_counter = 0;
	while (_counter < _maxCounts)
	{
		_delta = 0.0f;
		_framedelta = 0.0f;
		MPU_readSensor();
		_hxfilt = (_hxfilt * ((float) _coeff - 1) + (getMagX_uT() / _hxs + _hxb)) / ((float) _coeff);
		_hyfilt = (_hyfilt * ((float) _coeff - 1) + (getMagY_uT() / _hys + _hyb)) / ((float) _coeff);
		_hzfilt = (_hzfilt * ((float) _coeff - 1) + (getMagZ_uT() / _hzs + _hzb)) / ((float) _coeff);
		if (_hxfilt > _hxmax)
		{
			_delta = _hxfilt - _hxmax;
			_hxmax = _hxfilt;
		}
		if (_delta > _framedelta)
		{
			_framedelta = _delta;
		}
		if (_hyfilt > _hymax)
		{
			_delta = _hyfilt - _hymax;
			_hymax = _hyfilt;
		}
		if (_delta > _framedelta)
		{
			_framedelta = _delta;
		}
		if (_hzfilt > _hzmax)
		{
			_delta = _hzfilt - _hzmax;
			_hzmax = _hzfilt;
		}
		if (_delta > _framedelta)
		{
			_framedelta = _delta;
		}
		if (_hxfilt < _hxmin)
		{
			_delta = abs(_hxfilt - _hxmin);
			_hxmin = _hxfilt;
		}
		if (_delta > _framedelta)
		{
			_framedelta = _delta;
		}
		if (_hyfilt < _hymin)
		{
			_delta = abs(_hyfilt - _hymin);
			_hymin = _hyfilt;
		}
		if (_delta > _framedelta)
		{
			_framedelta = _delta;
		}
		if (_hzfilt < _hzmin)
		{
			_delta = abs(_hzfilt - _hzmin);
			_hzmin = _hzfilt;
		}
		if (_delta > _framedelta)
		{
			_framedelta = _delta;
		}
		if (_framedelta > _deltaThresh)
		{
			_counter = 0;
		}
		else
		{
			_counter++;
		}
		HAL_Delay(20);
	}

	// find the magnetometer bias
	_hxb = (_hxmax + _hxmin) / 2.0f;
	_hyb = (_hymax + _hymin) / 2.0f;
	_hzb = (_hzmax + _hzmin) / 2.0f;

	// find the magnetometer scale factor
	_hxs = (_hxmax - _hxmin) / 2.0f;
	_hys = (_hymax - _hymin) / 2.0f;
	_hzs = (_hzmax - _hzmin) / 2.0f;
	_avgs = (_hxs + _hys + _hzs) / 3.0f;
	_hxs = _avgs / _hxs;
	_hys = _avgs / _hys;
	_hzs = _avgs / _hzs;

	// set the srd back to what it was
	if (setSrd(_srd) < 0)
	{
		return -2;
	}
	return 1;
}

/* returns the magnetometer bias in the X direction, uT */
float getMagBiasX_uT()
{
	return _hxb;
}

/* returns the magnetometer scale factor in the X direction */
float getMagScaleFactorX()
{
	return _hxs;
}

/* returns the magnetometer bias in the Y direction, uT */
float getMagBiasY_uT()
{
	return _hyb;
}

/* returns the magnetometer scale factor in the Y direction */
float getMagScaleFactorY()
{
	return _hys;
}

/* returns the magnetometer bias in the Z direction, uT */
float getMagBiasZ_uT()
{
	return _hzb;
}

/* returns the magnetometer scale factor in the Z direction */
float getMagScaleFactorZ()
{
	return _hzs;
}

/* sets the magnetometer bias (uT) and scale factor in the X direction */
void setMagCalX(float bias, float scaleFactor)
{
	_hxb = bias;
	_hxs = scaleFactor;
}

/* sets the magnetometer bias (uT) and scale factor in the Y direction */
void setMagCalY(float bias, float scaleFactor)
{
	_hyb = bias;
	_hys = scaleFactor;
}

/* sets the magnetometer bias (uT) and scale factor in the Z direction */
void setMagCalZ(float bias, float scaleFactor)
{
	_hzb = bias;
	_hzs = scaleFactor;
}

static int8_t MPU_readRegisters(uint8_t subAddress, uint8_t count, uint8_t *data)
{
	return IIC_ReadRegisters(MPU_ADDRESS, subAddress, count, data);
}

static int8_t MPU_writeRegister(uint8_t subAddress, uint8_t data)
{
	IIC_WriteRegister(MPU_ADDRESS, subAddress, 1, data);
	HAL_Delay(10);
	// read the register and confirm
	if (MPU_readRegisters(subAddress, 1, _buffer) < 0)
	{
		return -1;
	}
	if (_buffer[0] == data)
	{
		return 0;
	}
	return -2;
}

/* writes a register to the AK8963 given a register address and data */
static int8_t MPU_writeAK8963Register(uint8_t subAddress, uint8_t data)
{
	// set slave 0 to the AK8963 and set for write
	if (MPU_writeRegister(I2C_SLV0_ADDR, AK8963_I2C_ADDR) < 0)
	{
		return -1;
	}
	// set the register to the desired AK8963 sub address
	if (MPU_writeRegister(I2C_SLV0_REG, subAddress) < 0)
	{
		return -2;
	}
	// store the data for write
	if (MPU_writeRegister(I2C_SLV0_DO, data) < 0)
	{
		return -3;
	}
	// enable I2C and send 1 byte
	if (MPU_writeRegister(I2C_SLV0_CTRL, I2C_SLV0_EN | (uint8_t) 1) < 0)
	{
		return -4;
	}
	// read the register and confirm
	if (MPU_readAK8963Registers(subAddress, 1, _buffer) < 0)
	{
		return -5;
	}
	if (_buffer[0] == data)
	{
		return 1;
	}
	else
	{
		return -6;
	}
}

/* reads registers from the AK8963 */
static int8_t MPU_readAK8963Registers(uint8_t subAddress, uint8_t count, uint8_t *dest)
{
	// set slave 0 to the AK8963 and set for read
	if (MPU_writeRegister(I2C_SLV0_ADDR, AK8963_I2C_ADDR | I2C_READ_FLAG) < 0)
	{
		return -1;
	}
	// set the register to the desired AK8963 sub address
	if (MPU_writeRegister(I2C_SLV0_REG, subAddress) < 0)
	{
		return -2;
	}
	// enable I2C and request the bytes
	if (MPU_writeRegister(I2C_SLV0_CTRL, I2C_SLV0_EN | count) < 0)
	{
		return -3;
	}
	HAL_Delay(1); // takes some time for these registers to fill
	// read the bytes off the MPU9250 EXT_SENS_DATA registers
	_status = MPU_readRegisters(EXT_SENS_DATA_00, count, dest);
	return _status;
}
