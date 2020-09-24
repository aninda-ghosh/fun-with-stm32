/*
 * quaternion.h
 *
 *  Created on: Sep 23, 2020
 *      Author: aninda
 */

#ifndef INC_QUATERNION_H_
#define INC_QUATERNION_H_


#include "math.h"


float roll;
float pitch;
float yaw;



// Function declarations
void madwickBegin(void);
void madwickUpdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void madwickGetAngles(float* angles);
float madwickGetRoll(void);
float madwickGetPitch(void);
float madwickGetYaw(void);
float madwickGetRollRadians(void);
float madwickGetPitchRadians(void);
float madwickGetYawRadians(void);




#endif /* INC_QUATERNION_H_ */
