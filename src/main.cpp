/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Arduino.h"
#include "secrets.h"

int16_t parseInt(String str)
{
	// char x = str[0];
	uint8_t i = 0;
	while (i < str.length() && !isdigit(str[i]) && str[i] != '-')
	{
		i++;
	}
	bool neg = false;
	if (str[i] == '-')
	{
		neg = true;
		i++;
	}
	int16_t num = 0;
	while (i < str.length() && isdigit(str[i]))
	{
		uint8_t digit = uint8_t(str[i] - '0');
		num *= 10;
		num += digit;
		i++;
	}
	return neg ? -num : num;
}

struct Pin
{
    uint8_t at;
    bool out;
    bool up;
};

Pin pins[4] = {
    Pin{D0, true, false},
    Pin{D2, true, false},
    Pin{D1, true, false},
    Pin{D3, true, false}};

ESP8266WebServer server;

String statusText() {
    String res = "[";
    for (auto i = 0; i < 4; i++) {
        if (i > 0) {
            res += ", ";
        }
        char buffer[200];
        sprintf(buffer, "{\"at\": %d, \"out\": %d, \"up\": %d}\n", pins[i].at, pins[i].out, pins[i].up);
        res += buffer;
    }
    res += "]";
    return res;
}

void web_hello()
{
	auto url = server.uri();
	server.sendHeader("Access-Control-Allow-Origin", "*");

    String res = "<body>";

    if (url.startsWith("/pin/")) {
        auto rest = url.substring(5);
        auto slash = rest.indexOf('/');
        auto pin = parseInt(rest.substring(0, slash));
        auto at = pins[pin].at;
        auto cmd = rest.substring(slash + 1);
        if (cmd.equals("out")) {
            pinMode(at, OUTPUT);
            pins[pin].out = true;
            res += "ok out";
            // server.send(200, "text/plain", "ok out");
            // return;
        }
        if (cmd.equals("in")) {
            pinMode(at, INPUT);
            pins[pin].out = false;
            res += "ok in";
            // server.send(200, "text/plain", "ok in");
            // return;
        }
        if (cmd.equals("up")) {
            if (pins[pin].out) {
                digitalWrite(at, 1);
                pins[pin].up = true;
                // server.send(200, "text/plain", "ok wrote");
                // return;
            } 
            // server.send(200, "text/plain", "not out");
            // return;
        }
        if (cmd.equals("down")) {
            if (pins[pin].out) {
                digitalWrite(at, 0);
                pins[pin].up = false;
                // server.send(200, "text/plain", "ok wrote");
                // return;
            }
            // server.send(200, "text/plain", "not out");
            // return;
        }
        // server.send(200, "application/json", statusText());
        // return;
    }

    for (auto i = 0; i < 4; i++) {
        char buffer[200];
        sprintf(buffer, "<br>Pin %d = <a href=\"/pin/%d/out\">Out</a> <a href=\"/pin/%d/in\">In</a>\n", i, i, i);
        res += buffer;
        if (pins[i].out) {
            if (pins[i].up) {
                char buffer[200];
                sprintf(buffer, "<a href=\"/pin/%d/down\">Down</a>\n", i);
                res += buffer;
            } else {
                char buffer[200];
                sprintf(buffer, "<a href=\"/pin/%d/up\">Up</a>\n", i);
                res += buffer;
            }
        }
    }
    res += statusText();
    server.send(200, "text/html", res);

    digitalWrite(LED_BUILTIN, 1);
    delay(100);
    digitalWrite(LED_BUILTIN, 0);

	// server.send(
	// 	200, "application/json",
	// 	"{\"x\":" + showArm(&armX) +
	// 		", \"y\":" + showArm(&armY) +
	// 		", \"z\":" + showArm(&armZ) +
	// 		", \"state\": " + showState(&state) + "}");
}

void setup()
{
    // initialize LED digital pin as an output.
    pinMode(LED_BUILTIN, OUTPUT);
	// from secrets.h
	WiFi.begin(WIFI_SSID, WIFI_PWD);

    for (auto i = 0; i < 4; i++) {
        pinMode(pins[i].at, OUTPUT);
    }

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

void loop()
{
	server.handleClient();

    // // turn the LED on (HIGH is the voltage level)
    // digitalWrite(LED_BUILTIN, HIGH);
    // // wait for a second
    // delay(1000);
    // // turn the LED off by making the voltage LOW
    // digitalWrite(LED_BUILTIN, LOW);
    // // wait for a second
    // delay(1000);
}