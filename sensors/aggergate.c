#include "aggergate.h"

#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <libNEMA.h>

static int FD_IMU;

//-----------------------------------------------------------------------------
static int openSensor(const char* dev, int* fd, int flags)
{
	*fd = open(dev, flags);
	return fd > 0;
}
//-----------------------------------------------------------------------------
int senInit(const char* imuDevice, const char* gpsDevice, const char* calProfile)
{
	printf("Initializing GPS...");
	if(gpsInit(gpsDevice)){
		printf("Failed!\n");
		return -1;
	}
	printf("OK!\n");

	printf("Initializing IMU...");
	if(!openSensor(imuDevice, &FD_IMU, O_RDWR)){
		printf("Failed!\n");
		return -2;
	}
	printf("OK!\n");

	if(calProfile){
		int calFd = open(calProfile, O_RDONLY);

		if(calFd <= 0 || imuLoadCalibrationProfile(calFd, &SYS.body.imu)){
			close(calFd);
			return -3;
		}

		close(calFd);
	}

	return 0;
}
//-----------------------------------------------------------------------------
int senShutdown()
{
	if(gpsShutdown()) return -1;
	if(close(FD_IMU)) return -2;

	return 0;
}
//-----------------------------------------------------------------------------
int senUpdate(fusedObjState_t* body)
{
	float dt = SYS.timeUp - body->lastMeasureTime;
	objectState_t *measured  = &body->measured;
	objectState_t *estimated = &body->estimated;

	imuUpdateState(FD_IMU, &body->imu);

	// update the heading according to magnetometer readings
	measured->heading.x = -body->imu.adjReadings.mag.x;
	measured->heading.y = -body->imu.adjReadings.mag.y;
	measured->heading.z =  body->imu.adjReadings.mag.z;
	measured->heading = vec3fNorm(&measured->heading);

	// use the gyro to estimate how confident we should be in the magnetometer's
	// current measured heading
	vec3f_t lastHeading = estimated->heading;
	vec3f_t gyroHeading = lastHeading;
	vec2fRot((vec2f_t*)&gyroHeading, (vec2f_t*)&measured->heading, body->imu.adjReadings.rotational.z * dt);

	float coincidence = vec3fDot(&lastHeading, &gyroHeading);
	vec3Lerp(estimated->heading, gyroHeading, measured->heading, coincidence);

	if(gpsHasNewReadings()){
		vec3f_t lastPos = measured->position;

		// assign new ements
		vec3f_t* velLin = &measured->velocity.linear;
		body->hasGpsFix = gpsGetReadings(&measured->position, velLin);
		vec3Sub(*velLin, measured->position, lastPos);
		vec3Scl(*velLin, *velLin, dt);

		// since we now have measurements, reset the estimates
		*estimated = *measured;

		body->lastMeasureTime = SYS.timeUp;
		body->lastEstTime     = SYS.timeUp;
	}
	else
	{
		float dt = SYS.timeUp - body->lastEstTime;

		// integrate position using velocity
		vec3f_t* estVelLin = &estimated->velocity.linear;
		estimated->position.x += estVelLin->x * dt;
		estimated->position.y += estVelLin->y * dt;
		estimated->position.z += estVelLin->z * dt;

		// integrate IMU acceleration into velocity
		estVelLin->x += body->imu.adjReadings.linear.x * dt;
		estVelLin->y += body->imu.adjReadings.linear.y * dt;
		estVelLin->z += body->imu.adjReadings.linear.z * dt;

		body->lastEstTime = SYS.timeUp;
	}
	return 0;
}
