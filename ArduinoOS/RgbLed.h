/*
Transistor: 330 Ohm
*/
#pragma once
#include "stdint.h"
#include "BaseLed.h"
class RgbLed : public BaseLed
{
private:
	bool isOn;
	uint8_t redPin;
	uint8_t greenPin;
	uint8_t bluePin;
	uint8_t redValue;
	uint8_t greenValue;
	uint8_t blueValue;
public:
	bool IsOn();
	void SetPins(uint8_t red, uint8_t green, uint8_t blue);
	void SetPins(uint8_t red, uint8_t green, uint8_t blue, bool resetPinMode);
	uint8_t GetRedPin();
	void SetRedPin(uint8_t pin);
	void SetRedPin(uint8_t pin, bool resetPinMode);
	uint8_t GetGreenPin();
	void SetGreenPin(uint8_t pin);
	void SetGreenPin(uint8_t pin, bool resetPinMode);
	uint8_t GetBluePin();
	void SetBluePin(uint8_t pin);
	void SetBluePin(uint8_t pin, bool resetPinMode);

	uint8_t GetRed();
	void SetRed(uint8_t redValue);
	uint8_t GetGreen();
	void SetGreen(uint8_t greenValue);
	uint8_t GetBlue();
	void SetBlue(uint8_t blueValue);
	void SetRGB(uint8_t red, uint8_t green, uint8_t blue);

	RgbLed(uint8_t redPin, uint8_t greenPin, uint8_t bluePin);
	~RgbLed();
	void ChangeState(bool on);
	void TurnOn();
	void TurnOff();
	void Reset();
};

