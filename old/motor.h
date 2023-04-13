#ifndef PRIMARY_HEADER_H
#define PRIMARY_HEADER_H
#include <Arduino.h>

void initializePins(uint8_t _pin[4]);
void setOutputPins(uint8_t _pin[4], uint8_t mask);
void step8(uint8_t _pin[4], long step);

struct Arm
{
	uint8_t pins[4];
	long logical;
	long dest;
	long last;
	long wait;
};
void initialize(Arm *arm);
void sendStep(Arm *arm);
void goToward(Arm *arm, long step);
void advance(Arm *arm, long now);

#endif