#include "mains.h"
#include "Arduino.h"

void setup()
{
	// motor_setup();
	// echo_setup();
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, 0);
	web_setup();
	digitalWrite(LED_BUILTIN, 1);
}

void loop()
{
	// motor_loop();
	web_loop();
	// echo_loop();
}