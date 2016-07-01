#include "Led.h"
#include <Arduino.h>

uint8_t Led::GetPin()
{
	return pin;
}

void Led::SetPin(uint8_t pin)
{
	SetPin(pin, true);
}

void Led::SetPin(uint8_t pin, bool resetPinMode)
{
	if (resetPinMode)
	{
		pinMode(this->pin, INPUT);
	}
	this->pin = pin;
	pinMode(pin, OUTPUT);
	if (IsOn())
	{
		analogWrite(pin, currentValue);
	}
}

Led::Led(uint8_t pin)
{
	currentValue = 255;
	isOn = false;
	SetPin(pin, false);
}

Led::~Led()
{
}

void Led::ChangeState(bool on)
{
	SetValue(on ? 255 : 0);
}

void Led::TurnOn()
{
	isOn = true;
	analogWrite(pin, currentValue);
}

void Led::TurnOff()
{
	isOn = false;
	digitalWrite(pin, LOW);
}

bool Led::IsOn()
{
	return isOn;
}

uint8_t Led::GetValue()
{
	return currentValue;
}

void Led::SetValue(uint8_t value)
{
	currentValue = value;

	if (IsOn())
	{
		analogWrite(pin, currentValue);
	}
}