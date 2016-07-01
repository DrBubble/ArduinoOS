#include "RgbLed.h"
#include <Arduino.h>

bool RgbLed::IsOn()
{
	return isOn;
}

void RgbLed::SetPins(uint8_t red, uint8_t green, uint8_t blue)
{
	SetPins(red, green, blue, true);
}

void RgbLed::SetPins(uint8_t red, uint8_t green, uint8_t blue, bool resetPinMode)
{
	SetRedPin(redPin, true);
	SetGreenPin(greenPin, true);
	SetBluePin(bluePin, true);
}

uint8_t RgbLed::GetRedPin()
{
	return redPin;
}

void RgbLed::SetRedPin(uint8_t pin)
{
	SetRedPin(pin, true);
}

void RgbLed::SetRedPin(uint8_t pin, bool resetPinMode)
{
	if (resetPinMode)
	{
		pinMode(redPin, INPUT);
	}
	redPin = pin;
	pinMode(redPin, OUTPUT);
	if (IsOn())
	{
		analogWrite(redPin, redValue);
	}
}

uint8_t RgbLed::GetGreenPin()
{
	return greenPin;
}

void RgbLed::SetGreenPin(uint8_t pin)
{
	SetGreenPin(pin, true);
}

void RgbLed::SetGreenPin(uint8_t pin, bool resetPinMode)
{
	if (resetPinMode)
	{
		pinMode(greenPin, INPUT);
	}
	greenPin = pin;
	pinMode(greenPin, OUTPUT);
	if (IsOn())
	{
		analogWrite(greenPin, greenValue);
	}
}

uint8_t RgbLed::GetBluePin()
{
	return bluePin;
}

void RgbLed::SetBluePin(uint8_t pin)
{
	SetBluePin(pin, true);
}

void RgbLed::SetBluePin(uint8_t pin, bool resetPinMode)
{
	if (resetPinMode)
	{
		pinMode(bluePin, INPUT);
	}
	bluePin = pin;
	pinMode(bluePin, OUTPUT);
	if (IsOn())
	{
		analogWrite(bluePin, blueValue);
	}
}

uint8_t RgbLed::GetRed()
{
	return redValue;
}

void RgbLed::SetRed(uint8_t redValue)
{
	this->redValue = redValue;
	if (IsOn())
	{
		analogWrite(redPin, redValue);
	}
}

uint8_t RgbLed::GetGreen()
{
	return greenValue;
}

void RgbLed::SetGreen(uint8_t greenValue)
{
	this->greenValue = greenValue;
	if (IsOn())
	{
		analogWrite(greenPin, greenValue);
	}
}

uint8_t RgbLed::GetBlue()
{
	return blueValue;
}

void RgbLed::SetBlue(uint8_t blueValue)
{
	this->blueValue = blueValue;
	if (IsOn())
	{
		analogWrite(bluePin, blueValue);
	}
}

void RgbLed::SetRGB(uint8_t red, uint8_t green, uint8_t blue)
{
	SetRed(red);
	SetGreen(green);
	SetBlue(blue);
}

RgbLed::RgbLed(uint8_t redPin, uint8_t greenPin, uint8_t bluePin)
{
	isOn = false;
	redValue = 0;
	greenValue = 0;
	blueValue = 0;
	SetRedPin(redPin, false);
	SetGreenPin(greenPin, false);
	SetBluePin(bluePin, false);
}


RgbLed::~RgbLed()
{
}

void RgbLed::ChangeState(bool on)
{
	if (on)
	{
		TurnOn();
	}
	else
	{
		TurnOff();
	}
}

void RgbLed::TurnOn()
{
	isOn = true;
	analogWrite(redPin, redValue);
	analogWrite(greenPin, greenValue);
	analogWrite(bluePin, blueValue);
}

void RgbLed::TurnOff()
{
	isOn = false;
	digitalWrite(redPin, LOW);
	digitalWrite(greenPin, LOW);
	digitalWrite(bluePin, LOW);
}

void RgbLed::Reset()
{
	TurnOff();
	redValue = 0;
	greenValue = 0;
	blueValue = 0;
}