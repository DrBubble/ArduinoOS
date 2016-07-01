#pragma once
#include <stdint.h>
class ISupportsSinglePin
{
public:
	virtual uint8_t GetPin() = 0;
	virtual void SetPin(uint8_t pin) = 0;
	virtual void SetPin(uint8_t pin, bool resetPinMode) = 0;
};

