/*
Transistor: 330 Ohm
*/
#pragma once
#include "stdint.h"
#include "BaseLed.h"
#include "IValueable.h"
#include "ISupportsSinglePin.h"
class Led : public BaseLed, public IValueable, public ISupportsSinglePin
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
	Led(uint8_t pin);
	~Led();
	void ChangeState(bool on);
	void TurnOn();
	void TurnOff();
	uint8_t GetValue();
	void SetValue(uint8_t value);
};

