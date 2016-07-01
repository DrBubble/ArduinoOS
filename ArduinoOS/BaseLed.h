#pragma once
#include "stdint.h"
#include "ISwitchable.h"
class BaseLed : public ISwitchable
{
public:
	virtual bool IsOn() = 0;
};

