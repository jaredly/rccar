#include "Arduino.h"

void echo_setup()
{
	Serial.begin(9600);
}

void echo_loop()
{
	if (Serial.available())
	{
		auto text = Serial.readStringUntil('\n');
		Serial.println(text);
	}
}
