/**
 * v1
 *
 * Thinking about simpler stuff.
 */
#include "motor.h"
#include "logic.h"
#include "./cmds.h"

#define IN1 D5
#define IN2 D6
#define IN3 D7
#define IN4 D8

struct Config
{
	int steps;
	float rpm;
	int backlash;
	int wait;
	bool useBacklash;
};

struct State
{
	Arm arm1;
	Arm arm2;
	Arm arm3;
};

State mstate = State{
	Arm{{D0, D2, D1, D3}},
	Arm{{D4, D6, D5, D7}},
	// no 6,7,8,11
	Arm{{11, 12, D8, A0}},
};

// nano attempt
// Arm{{9, 10, 11, 12}, 0, 0, 0, false},
// Arm{{5, 6, 7, 8}, 0, 0, 0, false}};

// Old nodemcu
// Arm{{IN1, IN3, IN2, IN4}, 0, 0, 0, false},
// Arm{{D1, D3, D2, D4}, 0, 0, 0, false}};

long last = 0;
struct Dest
{
	long arm1;
	long arm2;
	long arm3;
	bool done;
};
Dest dest = Dest{0, 0, 0};
Config config = Config{4076, 100.0, 40, 20000, true};

void motor_setup()
{
	Serial.begin(9600);
	last = micros();
	// initialize
	initialize(&mstate.arm1);
	initialize(&mstate.arm2);

	// pinMode()
	initialize(&mstate.arm3);
	// sendStep(&mstate.arm1);
}

int angleToSteps(double angle)
{
	int total = 4076;
	return int(total * angle / PI / 2.0);
}

void sendStatus()
{
	char buffer[200];
	sprintf(buffer, "{\"arm1\": %ld, \"arm2\": %ld, \"dest\": [%ld, %ld]}\n",
			mstate.arm1.logical,
			mstate.arm2.logical,
			dest.arm1, dest.arm2);
	Serial.print(buffer);
}

// Very basic stuff here.
void motor_loop()
{
	// 10ms?
	long wait = config.wait;
	long now = micros();
	if (now - last > wait)
	{
		last = now;
		goToward(&mstate.arm1, dest.arm1);
		goToward(&mstate.arm2, dest.arm2);

		if (!dest.done && mstate.arm1.logical == dest.arm1 &&
			mstate.arm2.logical == dest.arm2)
		{
			dest.done = true;
			Serial.println("Done");
			sendStatus();
		}
	}
	// if (Serial.available())
	// {
	// 	auto text = Serial.readStringUntil('\n');
	// 	Serial.println(text);
	// 	// cmd cmd;
	// 	// parseCmd(&cmd, text);
	// 	// // if (text.equals("ping"))
	// 	// // {
	// 	// // 	Serial.println("pong");
	// 	// // 	return;
	// 	// // }

	// 	// // DynamicJsonDocument doc(1024);
	// 	// // deserializeJson(doc, text);

	// 	// // if (doc.isNull())
	// 	// // {
	// 	// // 	Serial.println("Unable to parse json");
	// 	// // 	return;
	// 	// // }
	// 	// // int cmd = doc["cmd"];
	// 	// switch (cmd.id)
	// 	// {
	// 	// case ping:
	// 	// 	Serial.println("pong");
	// 	// 	return;
	// 	// case move:
	// 	// {
	// 	// 	dest.arm1 = cmd.int1;
	// 	// 	dest.arm2 = cmd.int2;
	// 	// 	dest.done = false;
	// 	// 	char buffer[50];
	// 	// 	sprintf(buffer, "Going to arm1=%ld, arm2=%ld\n", dest.arm1, dest.arm2);
	// 	// 	Serial.print(buffer);
	// 	// 	break;
	// 	// }
	// 	// // case 2:
	// 	// // {
	// 	// // 	double x = doc["x"];
	// 	// // 	double y = doc["y"];
	// 	// // 	Pos angles = armAngles(Pos{x, y}, true);
	// 	// // 	long arm1 = angleToSteps(angles.x);
	// 	// // 	long arm2 = angleToSteps(angles.y);
	// 	// // 	char buffer[50];
	// 	// // 	sprintf(buffer, "Going to x=%f:%ld:%f, y=%f:%ld:%f\n",
	// 	// // 			x, arm1, angles.x / PI * 180.0,
	// 	// // 			y, arm2, angles.y / PI * 180.0);
	// 	// // 	Serial.print(buffer);
	// 	// // 	if (!doc["sim"])
	// 	// // 	{
	// 	// // 		dest.arm1 = arm1;
	// 	// // 		dest.arm2 = arm2;
	// 	// // 		dest.done = false;
	// 	// // 	}
	// 	// // 	break;
	// 	// // }
	// 	// case status:
	// 	// {
	// 	// 	sendStatus();
	// 	// 	break;
	// 	// }
	// 	// case reset:
	// 	// {
	// 	// 	mstate.arm1.physical = 0;
	// 	// 	mstate.arm1.logical = 0;
	// 	// 	mstate.arm2.physical = 0;
	// 	// 	mstate.arm2.logical = 0;
	// 	// 	dest.arm1 = 0;
	// 	// 	dest.arm2 = 0;
	// 	// 	Serial.println("Reset!");
	// 	// 	break;
	// 	// }
	// 	// case setWait:
	// 	// {
	// 	// 	// if (doc.containsKey("backlash"))
	// 	// 	// {
	// 	// 	// 	config.backlash = doc["backlash"];
	// 	// 	// }
	// 	// 	// if (doc.containsKey("steps"))
	// 	// 	// {
	// 	// 	// 	config.steps = doc["steps"];
	// 	// 	// }
	// 	// 	// if (doc.containsKey("rpm"))
	// 	// 	// {
	// 	// 	// 	config.rpm = doc["rpm"];
	// 	// 	// }
	// 	// 	// if (doc.containsKey("wait"))
	// 	// 	// {
	// 	// 	config.wait = cmd.int1;
	// 	// 	// }
	// 	// 	// if (doc.containsKey("useBacklash"))
	// 	// 	// {
	// 	// 	// 	config.useBacklash = doc["useBacklash"];
	// 	// 	// }
	// 	// 	break;
	// 	// }
	// 	// default:
	// 	// {
	// 	// 	char buffer[50];
	// 	// 	sprintf(buffer, "Unrecognized command: %d", cmd.id);
	// 	// 	Serial.println(buffer);
	// 	// 	Serial.println(text);
	// 	// }
	// 	// }
	// }
}