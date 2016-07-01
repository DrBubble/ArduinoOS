#pragma once
#include "stdint.h"
#include "BaseLed.h"
#include "IValueable.h"
#include "ISupportsSinglePin.h"
#include "ServoTimer2.h"
class Servo : public IValueable, public ISupportsSinglePin
{
private:
	uint8_t pin;
	uint8_t currentValue;
	ServoTimer2 baseServo;
public:
	bool IsOn();
	uint8_t GetPin();
	void SetPin(uint8_t pin);
	void SetPin(uint8_t pin, bool resetPinMode);
	Servo(uint8_t pin);
	~Servo();
	void ChangeState(bool on);
	void TurnOn();
	void TurnOff();
	uint8_t GetValue();
	void SetValue(uint8_t value);
};

