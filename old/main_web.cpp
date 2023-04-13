#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "./cmds.h"
#include "motor.h"
#include "secrets.h"

ESP8266WebServer server;

// Ground pizza
Arm armY = Arm{{D0, D2, D1, D3}, 0, 0, 0, 1000};
// Cross axis
Arm armX = Arm{{D5, D7, D6, D8}, 0, 0, 0, 1000};
// Pen lift
Arm armZ = Arm{{10, 3, 1, D4}, 0, 0, 0, 1000};

struct Step
{
	bool down;
	int16_t x;
	int16_t y;
	uint32_t minWait;
};

// Can go up to 4000 if needed.
const int MAX_STEPS = 2000;

struct Bound
{
	int max;
	int min;
};

struct State
{
	Step steps[MAX_STEPS];
	int current_step;
	int last_step;
	Bound xBound;
	Bound yBound;
	Bound zBound;
	int minWait;
};

String showState(State *state)
{
	char buffer[200];
	sprintf(buffer, "{\"bounds\": {\"x\": [%d, %d], \"y\": [%d, %d], \"z\": [%d, %d]}, \"steps\": [%d, %d]}",
			state->xBound.min,
			state->xBound.max,
			state->yBound.min,
			state->yBound.max,
			state->zBound.min,
			state->zBound.max,
			state->current_step,
			state->last_step);
	return String(buffer);
}

State state;

String showArm(Arm *arm)
{
	char buffer[100];
	sprintf(buffer, "{\"pos\": %ld, \"dest\": %ld, \"wait\": %ld}",
			arm->logical,
			arm->dest,
			arm->wait);
	return String(buffer);
}

bool is_on[18];

void nextStep()
{
	if (state.last_step == state.current_step)
	{
		return;
	}
	state.current_step = (state.current_step + 1) % MAX_STEPS;
	Step *step = &state.steps[state.current_step];
	int dx = armX.logical - step->x;
	int dy = armY.logical - step->y;
	if (abs(dx) > abs(dy))
	{
		armX.wait = step->minWait;
		if (dy != 0)
		{
			armY.wait = step->minWait * abs(dx) / abs(dy);
		}
	}
	else
	{
		armY.wait = step->minWait;
		if (dx != 0)
		{
			armX.wait = step->minWait * abs(dy) / abs(dx);
		}
	}
	armX.dest = step->x;
	armY.dest = step->y;
	armZ.dest = step->down ? state.zBound.min : state.zBound.max;
}

bool addStep(int x, int y, int minWait, bool down)
{
	int next = (state.last_step + 1) % MAX_STEPS;
	if (next == state.current_step)
	{
		// oops wraparound, drop this on the floor
		return true;
	}
	state.last_step = next;
	state.steps[next].down = down;
	state.steps[next].x = min(state.xBound.max, max(state.xBound.min, x));
	state.steps[next].y = min(state.yBound.max, max(state.yBound.min, y));
	// min of 2ms
	state.steps[next].minWait = min(max(state.minWait, minWait), 100000);
	return false;
}

void respondWithStatus()
{
	server.send(
		200, "application/json",
		"{\"x\":" + showArm(&armX) +
			", \"y\":" + showArm(&armY) +
			", \"z\":" + showArm(&armZ) +
			", \"state\": " + showState(&state) + "}");
}

void parseBounds(Bound *bounds, String url, int vmin, int vmax)
{
	auto under = url.indexOf('_', vmin);
	auto first = parseInt(url.substring(vmin, under));
	auto second = parseInt(url.substring(under + 1, vmax));
	bounds->min = min(first, second);
	bounds->max = max(first, second);
}

void web_hello()
{
	auto url = server.uri();

	server.sendHeader("Access-Control-Allow-Origin", "*");

	if (url.equals("/status"))
	{
		respondWithStatus();
		return;
	}

	if (url.startsWith("/bounds/"))
	{
		int at = 8;
		auto c1 = url.indexOf(',', at + 1);
		auto c2 = url.indexOf(',', c1 + 1);
		if (c1 != -1 && c2 != -1)
		{
			parseBounds(&state.xBound, url, at, c1);
			parseBounds(&state.yBound, url, c1 + 1, c2);
			parseBounds(&state.zBound, url, c2 + 2, url.length());
		}
		respondWithStatus();
		return;
	}

	if (url.startsWith("/stop"))
	{
		// Skip all the other steps
		state.current_step = state.last_step;
		armX.dest = armX.logical;
		armY.dest = armY.logical;
		armZ.dest = armZ.logical;
		respondWithStatus();
		return;
	}

	// x,y,minWait
	if (url.startsWith("/step/"))
	{
		int at = 6;
		uint8_t added = 0;
		while (at != -1)
		{
			auto next = url.indexOf('+', at + 1);
			if (next == -1)
			{
				next = url.length();
			}
			bool down = url[at] == 'd';
			auto c1 = url.indexOf(',', at + 1);
			auto c2 = url.indexOf(',', c1 + 1);
			if (c1 == -1)
			{
				break;
			}
			if (c2 == -1)
			{
				c2 = next;
			}
			bool finished = addStep(
				parseInt(url.substring(at, c1)),
				parseInt(url.substring(c1 + 1, c2)),
				parseInt(url.substring(min(c2 + 1, next), next)),
				down);
			if (finished)
			{
				break;
			}
			added++;
			at = next + 1;
		}
		char buffer[20];
		sprintf(buffer, "{\"steps\": %d}", added);
		server.send(200, "application/json", buffer);
		return;
	}

	if (url.startsWith("/arm/"))
	{
		auto rest = url.substring(5);
		auto slash = rest.indexOf('/');
		if (slash == -1)
		{
			auto c1 = rest.indexOf(',');
			auto c2 = rest.indexOf(',', c1 + 1);
			armX.dest = parseInt(rest.substring(0, c1));
			armY.dest = parseInt(rest.substring(c1, c2));
			armZ.dest = parseInt(rest.substring(c2));
		}
		else
		{
			auto which = rest.substring(0, slash);
			auto dest = parseInt(rest.substring(slash));
			if (which.equals("x"))
			{
				armX.dest = dest;
			}
			if (which.equals("y"))
			{
				armY.dest = dest;
			}
			if (which.equals("z"))
			{
				armZ.dest = dest;
			}
		}
		respondWithStatus();
		return;
	}

	if (url.startsWith("/wait/"))
	{
		auto rest = url.substring(6);
		auto slash = rest.indexOf('/');
		if (slash == -1)
		{
			auto c1 = rest.indexOf(',');
			auto c2 = rest.indexOf(',', c1 + 1);
			armX.wait = parseInt(rest.substring(0, c1));
			armY.wait = parseInt(rest.substring(c1, c2));
			armZ.wait = parseInt(rest.substring(c2));
		}
		else
		{
			auto which = rest.substring(0, slash);
			auto wait = parseInt(rest.substring(slash));
			if (which.equals("x"))
			{
				armX.wait = wait;
			}
			if (which.equals("y"))
			{
				armY.wait = wait;
			}
			if (which.equals("z"))
			{
				armZ.wait = wait;
			}
		}
		respondWithStatus();
		return;
	}

	auto data = "<body><h3>Hello folks</h3>\n" + server.uri() + "<br/>" +
				"<a href='/arm/0'>0</a><br/>" +
				"<a href='/arm/100'>100</a><br/>" +
				"<a href='/arm/-100'>-100</a><br/>" +
				"<a href='/arm/500'>500</a><br/>" +
				"ArmX: " + showArm(&armX) + "<br/>" +
				"ArmY: " + showArm(&armY) + "<br/>" +
				"ArmZ: " + showArm(&armZ) + "<br/>";

	server.send(200, "text/html", data);
}

void web_setup()
{
	Serial.begin(9600);
	// from secrets.h
	WiFi.begin(WIFI_SSID, WIFI_PWD);

	state.minWait = 1000;

	initialize(&armX);
	initialize(&armY);
	initialize(&armZ);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(1000);
		digitalWrite(LED_BUILTIN, 1);
		delay(100);
		digitalWrite(LED_BUILTIN, 0);
	}

	server.on("/", web_hello);
	server.onNotFound(web_hello);
	server.begin();
}

void web_loop()
{
	server.handleClient();
	auto now = micros();
	advance(&armX, now);
	advance(&armY, now);
	advance(&armZ, now);
	if (armX.dest == armX.logical && armY.dest == armY.logical && armZ.dest == armZ.logical)
	{
		nextStep();
	}
}