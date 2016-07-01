#include "Servo.h"
#include <Arduino.h>

uint8_t Servo::GetPin()
{
	return pin;
}

void Servo::SetPin(uint8_t pin)
{
	SetPin(pin, true);
}

void Servo::SetPin(uint8_t pin, bool resetPinMode)
{
	if (resetPinMode)
	{
		pinMode(this->pin, INPUT);
	}
	this->pin = pin;
	baseServo.attach(pin);
	baseServo.write(544 + currentValue * 1856.0 / 180);
}

Servo::Servo(uint8_t pin)
{
	currentValue = 90;
	SetPin(pin, false);
}

Servo::~Servo()
{
}

uint8_t Servo::GetValue()
{
	return currentValue;
}

void Servo::SetValue(uint8_t value)
{
	currentValue = value;

	baseServo.write(544 + currentValue * 1856.0 / 180);
}