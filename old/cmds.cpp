#include "./cmds.h"

void parseCmd(cmd *cmd, String text)
{
	if (text.equals("ping"))
	{
		cmd->id = ping;
		return;
	}
	if (text.equals("status"))
	{
		cmd->id = status;
		return;
	}
	if (text.equals("reset"))
	{
		cmd->id = reset;
		return;
	}
	if (text.startsWith("move:"))
	{
		auto rest = text.substring(5);
		auto mid = rest.indexOf(',');
		auto arm1 = rest.substring(0, mid);
		auto arm2 = rest.substring(mid + 1);
		cmd->id = move;
		cmd->int1 = parseInt(arm1);
		cmd->int2 = parseInt(arm2);
		return;
	}
	if (text.startsWith("set-wait:"))
	{
		auto rest = text.substring(9);
		cmd->id = move;
		cmd->int1 = parseInt(rest);
		return;
	}
}

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
