#include "motor.h"

void initializePins(uint8_t _pin[4])
{
	for (uint8_t i = 0; i < 4; i++)
		pinMode(_pin[i], OUTPUT);
}

void initialize(Arm *arm)
{
	initializePins(arm->pins);
}

void setOutputPins(uint8_t _pin[4], uint8_t mask)
{
	uint8_t numpins = 4;
	uint8_t i;
	for (i = 0; i < numpins; i++)
	{
		uint8_t value = (mask & (1 << i)) ? (HIGH) : (LOW);
		// Serial.printf("Pin %d %d\n", _pin[i], value);
		digitalWrite(_pin[i], value);
	}
}

void step8(uint8_t _pin[4], long step)
{
	switch (step & 0x7)
	{
	case 0: // 1000
		setOutputPins(_pin, 0b0001);
		break;

	case 1: // 1010
		setOutputPins(_pin, 0b0101);
		break;

	case 2: // 0010
		setOutputPins(_pin, 0b0100);
		break;

	case 3: // 0110
		setOutputPins(_pin, 0b0110);
		break;

	case 4: // 0100
		setOutputPins(_pin, 0b0010);
		break;

	case 5: // 0101
		setOutputPins(_pin, 0b1010);
		break;

	case 6: // 0001
		setOutputPins(_pin, 0b1000);
		break;

	case 7: // 1001
		setOutputPins(_pin, 0b1001);
		break;
	}
}

void sendStep(Arm *arm)
{
	step8(arm->pins, arm->logical);
}

void goToward(Arm *arm, long step)
{
	if (arm->logical != step)
	{
		int next = step > arm->logical ? 1 : -1;
		bool direction = step > arm->logical;
		arm->logical += next;
		sendStep(arm);
	}
}

void advance(Arm *arm, long now)
{
	if (now - arm->last > arm->wait)
	{
		arm->last = now;
		goToward(arm, arm->dest);
	}
}