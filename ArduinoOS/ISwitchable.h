#pragma once
class ISwitchable
{
public:
	virtual bool IsOn() = 0;
	virtual void TurnOn() = 0;
	virtual void TurnOff() = 0;
};

