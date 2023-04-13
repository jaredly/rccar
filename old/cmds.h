#ifndef CMDS
#define CMDS
#include <Arduino.h>

enum cmdId
{
	unknown,
	ping,
	status,
	move,
	reset,
	setWait
};

struct cmd
{
	cmdId id;
	int16_t int1;
	int16_t int2;
};

void parseCmd(cmd *cmd, String text);

int16_t parseInt(String str);

#endif