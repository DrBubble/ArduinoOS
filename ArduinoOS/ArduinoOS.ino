#include "KernelInitializer.h"

lock *serialLock = GetLockObject();

void setup()
{
	Serial.begin(9600);
	KernelInitializer::InitializeKernel(mainThread);
}

void mainThread()
{
	InitTask(secondThread);
	InitTaskWithStackSize(wastingCpuThread, STACK_SIZE_SMALL);
	while (true)
	{
		try
		{
			throw(EXCEPTION_ILLEGAL_ARGUMENT_NULL);
		}
		catch
		{
			AquireLock(serialLock);
			Serial.println("There was an Exception!");
			Serial.print("Error code: ");
			Serial.println(GetException());
			ReleaseLock(serialLock);
		}
		clearException();
		sleep(1000);
	}
}

void secondThread()
{
	while (true)
	{
		AquireLock(serialLock);
		Serial.println("Spam!");
		ReleaseLock(serialLock);
		sleep(1000);
	}
}

void wastingCpuThread()
{
	while (true)
	{
		// Nothing
	}
}