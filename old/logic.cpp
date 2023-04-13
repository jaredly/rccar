#include "logic.h"

Pos push(Pos pos, double theta, double mag)
{
	return Pos{
		pos.x + cos(theta) * mag,
		pos.y + sin(theta) * mag,
	};
}

double angleTo(Pos from, Pos to)
{
	return atan2(to.y - from.y, to.x - from.x);
}

double dist(Pos a, Pos b)
{
	double dx = a.x - b.x;
	double dy = a.y - b.y;
	return sqrt(dx * dx + dy * dy);
}

double S = 0.5;
double S2 = S * S;

// This is the Z of my setup. 12mm off of 75mm arm.
// If Z is variable, we can't precompute these things.
double Z = 12.0 / 75.0 / 2.0;
double Z2 = Z * Z;
double L2 = S2 + Z2;
double L = sqrt(L2);
double SL = S2 - L2;
double angleFromPen = acos(Z / L);

Pos origin = {x : 0, y : 0};

Pos armAnglesInner(Pos pen)
{
	double D2 = pen.x * pen.x + pen.y * pen.y;
	double D = sqrt(D2);
	if (D < Z)
	{
		D = Z * 1.01;
		D2 = D * D;
		pen = push(origin, angleTo(origin, pen), D);
	}
	double theta = acos((D2 + SL) / (2.0 * S * D)); // angle at origin
	double alpha = acos((D2 - SL) / (2.0 * L * D)); // angle at pen

	Pos T = push(pen, angleTo(pen, origin) - alpha + angleFromPen, Z);

	double angle1 = angleTo(origin, pen) + theta;
	double angle2 = angleTo(origin, T) * 2.0 - angle1;

	return {angle1, angle2};
};

Pos armAngles(Pos pen, bool leftArm)
{
	Pos response = armAnglesInner(
		leftArm
			? Pos{
				  x : -pen.x,
				  y : pen.y,
			  }
			: pen);

	if (leftArm)
	{
		return Pos{PI - response.x, PI - response.y};
	}
	return response;
};

Pos penPosInner(double angle1, double angle2)
{
	double mid = (angle1 + angle2) / 2.0;
	double off = angle1 - mid;
	double d = cos(off) * S * 2.0;
	Pos top = push(origin, mid, d);
	return push(top, angle2 + PI / 2.0, Z);
};

Pos penPos(double angle1, double angle2, bool leftArm)
{
	if (leftArm)
	{
		angle1 = PI - angle1;
		angle2 = PI - angle2;
	}
	Pos back = penPosInner(angle1, angle2);
	return leftArm ? Pos{x : -back.x, y : back.y} : back;
};
