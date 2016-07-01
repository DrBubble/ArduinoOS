#pragma once
#include "stdint.h"
class IValueable
{
public:
	virtual uint8_t GetValue() = 0;
	virtual void SetValue(uint8_t value) = 0;
};

