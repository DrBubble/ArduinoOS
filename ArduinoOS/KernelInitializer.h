/*
Memory usage:
No Features -> 1 Thread: 50 Byte + Stack size
All Features -> 1 Thread: 56 Byte + Stack size

Max Threads on device (1 System Thread):
Arduino Uno	 -> All Features: 20 Threads
No Features:  21 Threads

Arduino Mega -> All Features: 90 Threads
No Features:  97 Threads
*/
#pragma once
#include "TimerOne.h"

extern "C" {
#include "Kernel.h"
}


class KernelInitializer
{
public:
	static void InitializeKernel(void(*mainFunction)());
	static void InitializeKernel(void(*mainFunction)(), uint16_t stackSize);
	static void InitializeKernel(void(*mainFunction)(), uint16_t stackSize, long contextSwitchInterval);
private:
	static void HandleKernelInterrupt();
};

