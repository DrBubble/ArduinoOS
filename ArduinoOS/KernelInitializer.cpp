#include "KernelInitializer.h"
#include "Arduino.h"


void KernelInitializer::InitializeKernel(void(*mainFunction)())
{
	InitializeKernel(mainFunction, STACK_SIZE_LARGE);
}

void KernelInitializer::InitializeKernel(void(*mainFunction)(), uint16_t stackSize)
{
	InitializeKernel(mainFunction, stackSize, 2 * 1000l);
}

void KernelInitializer::InitializeKernel(void(*mainFunction)(), uint16_t stackSize, long contextSwitchInterval)
{
	InitializeKernelState((unsigned long)contextSwitchInterval);
	Timer1.initialize(contextSwitchInterval);
	InitTaskWithStackSizeAtomic(mainFunction, stackSize);
	Timer1.attachInterrupt(HandleKernelInterrupt);
	___calledFromInterrupt = false;
	SwitchContext();
}

void KernelInitializer::HandleKernelInterrupt()
{
	___calledFromInterrupt = true;
	SwitchContext();
}