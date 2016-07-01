#pragma once
#include "PiezoSpeaker.h"
#include <Arduino.h>

class Motor : public ISwitchable
{
private:
	bool isOn;
	bool hasHBridge;
	uint8_t pinPositive;
	uint8_t pinNegative;
	uint8_t pinHBridge;
	int16_t currentValue;
	void UpdateValue();
public:
	bool IsOn();
	uint8_t GetPinPositive();
	void SetPinPositive(uint8_t pinPositive);
	void SetPinPositive(uint8_t pinPositive, bool resetPinMode);
	uint8_t GetPinNegative();
	void SetPinNegative(uint8_t pinNegative);
	void SetPinNegative(uint8_t pinNegative, bool resetPinMode);
	uint8_t GetPinHBridge();
	void SetPinHBridge(uint8_t pinHBridge);
	void SetPinHBridge(uint8_t pinHBridge, bool resetPinMode);
	void SetPins(uint8_t pinPositive, uint8_t pinNegative);
	void SetPins(uint8_t pinPositive, uint8_t pinNegative, bool resetPinMode);
	void SetPins(uint8_t pinHBridge, uint8_t pinPositive, uint8_t pinNegative);
	void SetPins(uint8_t pinHBridge, uint8_t pinPositive, uint8_t pinNegative, bool resetPinMode);
	void SetPins(uint8_t pinHBridge, bool hasHBridge, uint8_t pinPositive, uint8_t pinNegative, bool resetPinMode);
	int16_t GetValue();
	void SetValue(int16_t value);
	uint8_t GetPositiveValue();
	void SetPositiveValue(uint8_t value);
	uint8_t GetNegativeValue();
	void SetNegativeValue(uint8_t value);
	bool HasHBridge();
	void SetHasHBridge(bool enable);
	void SetHasHBridge(bool enable, bool resetPinMode);
	void EnableHBridge();
	void DisableHBridge();
	void DisableHBridge(bool resetPinMode);
	Motor(uint8_t pinPositive, uint8_t pinNegative);
	Motor(uint8_t pinHBridge, uint8_t pinPositive, uint8_t pinNegative);
	~Motor();
	void TurnOn();
	void TurnOff();
};

