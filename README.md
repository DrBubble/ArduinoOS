# ArduinoOS
ArduinoOS is an operating system for arduino which supports multithreading and hardware abstaction.
## Table of Contents
1 [Setup](#id-Setup)<br>
&nbsp;&nbsp;&nbsp;&nbsp;1.1 [Usage](#id-Usage)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1.1.1 [Basics](#id-Basics)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1.1.1.1 [Sleep](#id-Sleep)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1.1.1.2 [Create Thread](#id-Create-Thread)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1.1.1.3 [Operating System uptime](#id-Operating-System- uptime)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1.1.1.4 [Locks](#id-Locks)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1.1.2 [Advanced](#id-Advanced)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1.1.2.1 [Error handling](#id-Error-handling)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1.1.2.1 [Kernel Panic](#id-Kernel-Panic)<br>
<div id='id-Setup'/>
## Setup
+ Download the latest release.
+ Copy the ArduinoOS folder to Documents\Arduino\libraries
+ Add the ArduinoOS library (You probably have to restart the Visual Studio or the Arduino IDE)
  + Visual Studio: Add Library -> User -> ArduinoOS
  + Arduino Studio: Sketch -> Include Library -> ArduinoOS
+ Replace your main file with:
```c++
#include "KernelInitializer.h"

void setup()
{
	KernelInitializer::InitializeKernel(mainThread);
}

void mainThread()
{

}
```
+ All code in the mainThread function will now be executed under the Operating System

<div id='id-Usage'/>
##Usage
<div id='id-Basics'/>
###Basics
<div id='id-Sleep'/>
####Sleep
In order to pause your programm you can use:
```c++
sleep(milliseconds);
```
Do not use ```delay``` since it does not allow the operating system to execute other tasks in the meanwhile and will block the thread for that time. That means ```delay(500)``` will block for 1 second if 2 Threads are running and for 2 seconds if 4 threads are running.
<div id='id-Create-Thread'/>
####Create Thread
With a ```InitTask ``` a new thread can be created.

Example:
```c++
void mainThread()
{
	InitTask(secondThread);
}

void secondThread()
{

}
```
<div id='id-Operating-System-uptime/>
####Operating System uptime
To get the uptime of the operating system you can use ```getPastMilliseconds```. Keep in mind that it will return a wrong value if you defined a kernel tick period that is not divisible through 1000 (1 ms).
<div id='id-Locks'/>
####Locks
In order to keep you application thread safe you can use locks. With locks you can prevent an other thread to access a variable, function, ... to be accessed in an unsafe state.

Example for conflicting threads:
``` c++
void mainThread()
{
	InitTask(thread2);
	while (true)
	{
		Serial.println("Thread1");
	}
}

void thread2()
{
	while (true)
	{
		Serial.println("Thread2");
	}
}
```
If you execute this code you will notice that it will output something strange like:
```
1hrThd2adTh
Tadea
T
eared1hrThd2adTh
Tadea
T
```
This is because while one thread is writing into the serial it will be interrupted by the other thread. In order to prevent this you can use locks. With the method lock ```GetLockObject``` you can create a instance of a lock object.

Example:
``` c++
lock *serialLock = GetLockObject();
```
With ```AquireLock(serialLock)```` you can now lock the object. To release it use ````ReleaseLock(serialLock)````.

Example:
``` c++
lock *serialLock = GetLockObject();

void mainThread()
{
	InitTask(thread2);
	while (true)
	{
		AquireLock(serialLock);
		Serial.println("Thread1");
		ReleaseLock(serialLock);
	}
}

void thread2()
{
	while (true)
	{
		AquireLock(serialLock);
		Serial.println("Thread2");
		ReleaseLock(serialLock);
	}
}
```
The output will now be like expected:
```
Thread1
Thread2
Thread1
Thread2
```
<div id='id-Advanced'/>
###Advanced
<div id='id-Error-handling'/>
####Error handling
<div id='id-Kernel Panic'/>
#####Kernel Panic
In order to catch kernel errors there is the function ````OnKernelPanic````. When this function gets called a kernel panic happened. A kernel panic is like a bluescreen in windows. When this function gets called the operating system is in an unstable state and stops its exection in order to prevent damage. Do not call any ArduinoOS functions inside this functions. They will not work and their result will be unpredictable. You can use this function for example to notify the user about the error code (LED, Serial, ...) or to reset the arduino.

Error codes:
````
KERNEL_ERROR_OUT_OF_STACK  = 1
KERNEL_ERROR_OUT_OF_MEMORY = 2
````

Example:
```` c++
void HandleKernelPanic(uint8_t errorCode)
{
	// User defined code for error handling
}

void setup()
{
	OnKernelPanic = HandleKernelPanic;
	KernelInitializer::InitializeKernel(mainThread);
}

void mainThread()
{
	// Provocates an out of memory error
	while (true)
	{
		InitTask(thread);
	}
}

void thread()
{
	while (true);
}
````
#####Free memory
In order to require the free memory use the function ````freeMemory````.
#####Free memory
In order to require the free stack use the function ````freeStack````. Keep in mind that this function will require a bit of stack itself.
####Stack
A Stack is a data type which allows putting (push) data on it and then take (pop) it from up to down. For more information see [Wikipedia](https://en.wikipedia.org/wiki/Stack_(abstract_data_type)). Various operatings like calling functions require space on the stack. ArduinoOS will reserve stack space for every thread. In order to influence the stack space use ```InitTaskWithStackSize``` or ```InitTaskWithStackSizeAndArgument``` when creating a new thread.

Example:
``` c++
void mainThread()
{
	InitTaskWithStackSize(thread2, STACK_SIZE_LARGE);
}

void thread2()
{

}
```

The predefined stack size values are:
```
STACK_SIZE_TINY = 64 bytes
STACK_SIZE_SMALL = 96 bytes
STACK_SIZE_MEDIUM = 128 bytes
STACK_SIZE_LARGE = 256 bytes
STACK_SIZE_GIANT = 512 bytes
STACK_SIZE_DEFAULT = 128 bytes
```
You can also pass you own stack size as an uint16_t. Keep in mind that this is not recommended for dying threads since the use of many different stack sizes can lead to stronger memory fragmentation. If a Thread with the stack size of 63 will be created 63 bytes of memory will be reserved. After that a second thread with 64 Bytes of reserved memory will be created. When the first thread gets destroyed 63 bytes of memory will be free before the second thread. If now a third thread with a stack size of 64 bytes will be created it can not be placed before the second thread since only 63 bytes of memory are free. So it will be placed after the second thread and will cause 63 bytes of memory to be wasted.

The main thread gets a default stack size of STACK_SIZE_LARGE. It can passed as a second argument in ````KernelInitializer::InitializeKernel````.

Example:
```` c++
KernelInitializer::InitializeKernel(mainThread, STACK_SIZE_MEDIUM);
````
