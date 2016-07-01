#include "Motor.h"

bool Motor::IsOn()
{
	return isOn;
}

uint8_t Motor::GetPinPositive()
{
	return pinNegative;
}

void Motor::SetPinPositive(uint8_t pinPositive)
{
	SetPinPositive(pinPositive, true);
}

void Motor::SetPinPositive(uint8_t pinPositive, bool resetPinMode)
{
	if (resetPinMode)
	{
		pinMode(this->pinPositive, INPUT);
	}
	this->pinPositive = pinPositive;
	pinMode(pinPositive, OUTPUT);
	UpdateValue();
}

uint8_t Motor::GetPinNegative()
{
	return pinNegative;
}

void Motor::SetPinNegative(uint8_t pinNegative)
{
	SetPinNegative(pinNegative, true);
}

void Motor::SetPinNegative(uint8_t pinNegative, bool resetPinMode)
{
	if (resetPinMode)
	{
		pinMode(this->pinNegative, INPUT);
	}
	this->pinNegative = pinNegative;
	pinMode(pinNegative, OUTPUT);
	UpdateValue();
}


uint8_t Motor::GetPinHBridge()
{
	return pinHBridge;
}

void Motor::SetPinHBridge(uint8_t pinHBridge)
{
	SetPinHBridge(pinHBridge, true);
}

void Motor::SetPinHBridge(uint8_t pinHBridge, bool resetPinMode)
{
	if (resetPinMode && hasHBridge)
	{
		pinMode(this->pinHBridge, INPUT);
	}
	this->pinHBridge = pinHBridge;
	pinMode(pinHBridge, OUTPUT);
	hasHBridge = true;
	UpdateValue();
}

void Motor::SetPins(uint8_t pinPositive, uint8_t pinNegative)
{
	SetPins(pinPositive, pinNegative, true);
}

void Motor::SetPins(uint8_t pinPositive, uint8_t pinNegative, bool resetPinMode)
{
	SetPins(pinHBridge, pinPositive, pinNegative, resetPinMode);
}

void Motor::SetPins(uint8_t pinHBridge, uint8_t pinPositive, uint8_t pinNegative)
{
	SetPins(pinHBridge, pinPositive, pinNegative, true);
}

void Motor::SetPins(uint8_t pinHBridge, uint8_t pinPositive, uint8_t pinNegative, bool resetPinMode)
{
	SetPins(pinHBridge, true, pinPositive, pinNegative, resetPinMode);
}

void Motor::SetPins(uint8_t pinHBridge, bool hashHBridge, uint8_t pinPositive, uint8_t pinNegative, bool resetPinMode)
{
	if (resetPinMode)
	{
		if (hasHBridge)
		{
			pinMode(this->pinHBridge, INPUT);
		}
		pinMode(this->pinPositive, INPUT);
		pinMode(this->pinNegative, INPUT);
	}
	this->pinHBridge = pinHBridge;
	this->pinPositive = pinPositive;
	this->pinNegative = pinNegative;
	pinMode(pinHBridge, OUTPUT);
	pinMode(pinPositive, OUTPUT);
	pinMode(pinNegative, OUTPUT);
	hasHBridge = hashHBridge;
	UpdateValue();
}

int16_t Motor::GetValue()
{
	return currentValue;
}

void Motor::SetValue(int16_t value)
{
	currentValue = value;
}

uint8_t Motor::GetPositiveValue()
{
	return currentValue > 0 ? currentValue : 0;
}

void Motor::SetPositiveValue(uint8_t value)
{
	currentValue = value;
}

uint8_t Motor::GetNegativeValue()
{
	return currentValue < 0 ? currentValue * -1 : 0;
}

void Motor::SetNegativeValue(uint8_t value)
{
	currentValue = value * -1;
}

bool Motor::HasHBridge()
{
	return hasHBridge;
}

void Motor::SetHasHBridge(bool enable)
{
	SetHasHBridge(enable, true);
}

void Motor::SetHasHBridge(bool enable, bool resetPinMode)
{
	if (hasHBridge)
	{
		pinMode(this->pinHBridge, INPUT);
	}
	hasHBridge = enable;
}

void Motor::EnableHBridge()
{
	SetHasHBridge(true);
}

void Motor::DisableHBridge()
{
	SetHasHBridge(false);
}

void Motor::DisableHBridge(bool resetPinMode)
{
	SetHasHBridge(false, resetPinMode);
}

Motor::Motor(uint8_t pinPositive, uint8_t pinNegative)
{
	isOn = false;
	currentValue = 255;
	SetPins(0, false, pinPositive, pinNegative, false);
}

Motor::Motor(uint8_t hBridge, uint8_t pinPositive, uint8_t pinNegative)
{
	isOn = false;
	currentValue = 255;
	SetPins(hBridge, pinPositive, pinNegative, false);
}

Motor::~Motor()
{

}

void Motor::TurnOn()
{
	isOn = true;
	UpdateValue();
}

void Motor::TurnOff()
{
	isOn = false;
	digitalWrite(pinPositive, LOW);
	digitalWrite(pinNegative, LOW);
}

void Motor::UpdateValue()
{
	if (IsOn())
	{
		if (hasHBridge)
		{
			if (currentValue == 0)
			{
				digitalWrite(pinHBridge, LOW);
				digitalWrite(pinPositive, LOW);
				digitalWrite(pinNegative, LOW);
			}
			else if (currentValue > 0)
			{
				analogWrite(pinHBridge, currentValue);
				digitalWrite(pinPositive, HIGH);
				digitalWrite(pinNegative, LOW);
			}
			else
			{

				analogWrite(pinHBridge, currentValue * -1);
				digitalWrite(pinPositive, LOW);
				digitalWrite(pinNegative, HIGH);
				digitalWrite(pinPositive, LOW);
			}
		}
		else
		{
			if (currentValue == 0)
			{
				digitalWrite(pinPositive, LOW);
				digitalWrite(pinNegative, LOW);
			}
			else if (currentValue > 0)
			{
				analogWrite(pinPositive, currentValue);
				digitalWrite(pinNegative, LOW);
			}
			else
			{
				digitalWrite(pinPositive, LOW);
				analogWrite(pinNegative, currentValue * -1);
			}
		}
	}
}