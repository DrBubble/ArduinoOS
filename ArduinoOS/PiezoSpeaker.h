#pragma once
#include "stdint.h"
#include "ISwitchable.h"
#include "IValueable.h"
#include "ISupportsSinglePin.h"
/*
Transistor: None
*/
class PiezoSpeaker : public ISwitchable, public IValueable, public ISupportsSinglePin
{
private:
	bool isOn;
	uint8_t pin;
	uint8_t currentValue;
public:
	bool IsOn();
	uint8_t GetPin();
	void SetPin(uint8_t pin);
	void SetPin(uint8_t pin, bool resetPinMode);
	uint8_t GetValue();
	void SetValue(uint8_t value);
	PiezoSpeaker(uint8_t pin);
	~PiezoSpeaker();
	void TurnOn();
	void TurnOff();
};

