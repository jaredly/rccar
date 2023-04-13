#ifndef LOGIC
#define LOGIC
#include <Arduino.h>

struct Pos
{
	double x;
	double y;
};

Pos push(Pos pos, double theta, double mag);
double angleTo(Pos from, Pos to);
double dist(Pos a, Pos b);

Pos armAngles(Pos pen, bool leftArm);
Pos penPos(double angle1, double angle2, bool leftArm);
#endif