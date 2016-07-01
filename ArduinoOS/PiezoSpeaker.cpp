#include "PiezoSpeaker.h"
#include <Arduino.h>

bool PiezoSpeaker::IsOn()
{
	return isOn;
}

uint8_t PiezoSpeaker::GetPin()
{
	return pin;
}

void PiezoSpeaker::SetPin(uint8_t pin)
{
	SetPin(pin, true);
}

void PiezoSpeaker::SetPin(uint8_t pin, bool resetPinMode)
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


uint8_t PiezoSpeaker::GetValue()
{
	return currentValue;
}

void PiezoSpeaker::SetValue(uint8_t value)
{
	currentValue = value;
	if (IsOn())
	{
		analogWrite(pin, currentValue);
	}
}

PiezoSpeaker::PiezoSpeaker(uint8_t pin)
{
	isOn = false;
	currentValue = 255;
	SetPin(pin, false);
}

PiezoSpeaker::~PiezoSpeaker()
{
}

void PiezoSpeaker::TurnOn()
{
	isOn = true;
	analogWrite(pin, currentValue);
}


void PiezoSpeaker::TurnOff()
{
	isOn = false;
	digitalWrite(pin, LOW);
}
