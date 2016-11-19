# ArduinoOS
ArduinoOS is an operating system for arduino which supports real multithreading, exceptions and hardware abstraction. The kernel of ArduinoOS is entirely written in C and optimized for minimal memory usage.

Max Threads on device (1 System Thread):<br>
Arduino Uno: 20 Threads<br>
Arduino Mega: 90 Threads

Example:
```` c++
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
	InitTaskWithStackSize(wastingCpuThread, STACK_SIZE_TINY);
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
````

## Table of Contents
1 [Setup](#id-Setup)<br>
2 [OS-Usage](#id-Usage)<br>
&nbsp;&nbsp;&nbsp;&nbsp;2.1 [Basics](#id-Basics)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.1 [Sleep](#id-Sleep)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.2 [Create Thread](#id-Create-Thread)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.3 [Operating System uptime](#id-Operating-System-uptime)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.4 [Locks](#id-Locks)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.5 [Exceptions](#id-Exceptions)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.5.1 [Throw](#id-Throw)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.5.2 [Try Catch](#id-Try-Catch)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.5.3 [Derivation](#id-Derivation)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.5.4 [Exception Codes](#id-Exception-Codes)<br>
&nbsp;&nbsp;&nbsp;&nbsp;2.2 [Advanced](#id-Advanced)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.2.1 [Error handling](#id-Error-handling)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.2.1.1 [Kernel Panic](#id-Kernel-Panic)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.2.1.2 [Free Memory](#id-Free-Memory)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.2.1.3 [Free Stack](#id-Free-Stack)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.2.2 [Stack](#id-Stack)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.2.3 [Thread Arguments](#id-Thread-Arguments)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.2.4 [Kernel Tick Period](#id-Kernel-Tick-Period)<br>
3 [Hardware abstraction](#id-Hardware-abstraction)<br>
<div id='id-Setup'/>
## Setup
+ Download the latest release.
+ Copy the ArduinoOS folder to Documents\Arduino\libraries
+ Add the ArduinoOS library (You probably have to restart the Visual Studio or the Arduino IDE)
  + Visual Studio: Add Library -> User -> ArduinoOS
  + Arduino Studio: Sketch -> Include Library -> ArduinoOS
+ Replace your main file with:
```c++
#include <KernelInitializer.h>

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
##OS-Usage
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
With ```InitTask ``` a new thread can be created.

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
<div id='id-Operating-System-uptime'/>
####Operating System uptime
To get the uptime of the operating system use ```getPastMilliseconds```.
<div id='id-Locks'/>
####Locks
In order to keep your application thread safe you can use locks. With locks you can prevent an other thread to access a variable, function, ... in an unsafe state.

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
This is because while one thread is writing into the serial it will be interrupted by the other thread. In order to prevent this you can use locks. With the method ```GetLockObject``` you can create a instance of a lock object.

Example:
``` c++
lock *serialLock = GetLockObject();
```
With ```AquireLock(serialLock)``` you can now lock the object. To release it use ````ReleaseLock(serialLock)````.

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
<div id='id-Exceptions'/>
####Exceptions
In order to make error handling more easy you can use exceptions. ArduinoOS provides simple integer based exceptions. To save memory there is no such thing as an error message. Uncaught exceptions will result in a [KernelPanic](#id-Kernel-Panic) with the error ````KERNEL_ERROR_UNHANDLED_EXCEPTION````.
<div id='id-Throw'/>
#####Throw
With ````throw```` a new exception can be thrown. This example throws an EXCEPTION_ILLEGAL_ARGUMENT_NULL. After the throw it will jump to the next catch that catches an exception of this type. If it does not get caught anywhere it will result in a [KernelPanic](#id-Kernel-Panic) with the error ````KERNEL_ERROR_UNHANDLED_EXCEPTION````.
``` c++
throw(EXCEPTION_ILLEGAL_ARGUMENT_NULL);
````
<div id='id-Try-Catch'/>
#####Try Catch
In order to catch exceptions use ````try```` and ````catch````. If an exception gets thrown inside the try block it will jump into the catch block. After the catch block ````clearException```` must be called. If you forget this there will be a syntax error.

Example:
```` c++
try
{
	// Try code
}
catch
{
	// Error handling code
}
clearException();
````
It is important that ````return```` can <b>not</b> be called inside a try block or catch block. There is some stack memory to free after the try catch block. Usually this is done inside clearException(). When returning inside a try block or catch block this function will never be called. In order to still return use ````retex````.

Example:
```` c++
try
{
	retex(someFunction()); // Save return
}
catch
{
	retex(-1); // Save return
}
clearException();
````

In order to catch exceptions of an specific type use ````catchType````.

Example:
```` c++
try
{
	// Try code
}
catchType(EXCEPTION_ILLEGAL_ARGUMENT)
{
	// Handle exceptions of type ILLEGAL_ARGUMENT 
}
clearException();
````

<div id='id-Derivation'/>
#####Derivation
Derivation means that a exception can have child and parent exceptions. When a parent exception gets caught inside a ````catchType```` also it child exceptions will be. All exceptions are derived from ````EXCEPTION````. Derivation is achieved by ranges starting with 50. The <b>0</b> at the end says it is a parent exception of all exceptions that have some other numbers instead of the 0. For example 5<b>0</b> is the parent exception from all exceptions from 5<b>1</b> to 5<b>9</b>.

Examples:
<pre>
0 (EXCEPTION) is the parent from all
1-49 have no children
5<b>0</b> is parent from 5<b>1</b> to 5<b>9</b>.
1<b>00</b> is parent from 1<b>01</b> to 1<b>99</b>.
1<b>000</b> is parent from 1<b>001</b> to 1<b>999</b>.
127<b>0</b> is parent from 127<b>1</b> to 127<b>9</b>.
</pre>

<div id='id-Exception-Codes'/>
#####Exception Codes
<pre>
EXCEPTION = 0
EXCEPTION_OUT_OF_STACK = 1
EXCEPTION_OUT_OF_MEMORY = 2

EXCEPTION_NULL_POINTER = 10
EXCEPTION_UNHANDLED_EXCEPTION = 11
EXCEPTION_NOT_IMPLEMENTED = 12
EXCEPTION_NOT_SUPORTED = 13
EXCEPTION_INVALID_FORMAT = 14
EXCEPTION_ACCES_VIOLATION = 15
EXCEPTION_ACCES_TIMEOUT = 16
EXCEPTION_INVALID_OPERATION_EXCEPTION = 50
EXCEPTION_ARITHMETIC = 60
EXCEPTION_ARITHMETIC_DIVIDE_BY_ZERO = 61
EXCEPTION_BUFFER_OVERFLOW = 70
EXCEPTION_INDEX_OUT_OF_RANGE = 71
EXCEPTION_ASSERTION_ERROR = 80
EXCEPTION_ILLEGAL_ARGUMENT = 90
EXCEPTION_ILLEGAL_ARGUMENT_NULL = 91
EXCEPTION_ILLEGAL_ARGUMENT_TO_BIG = 92
EXCEPTION_ILLEGAL_ARGUMENT_TO_SMALL = 93
EXCEPTION_ILLEGAL_ARGUMENT_OUT_OF_RANGE = 94
EXCEPTION_ILLEGAL_ARGUMENT_FORMAT = 95
EXCEPTION_IO = 110
EXCEPTION_IO_EOF = 111
EXCEPTION_IO_ACCESS_DENIED = 112
EXCEPTION_IO_INVALID_DATA = 113
EXCEPTION_IO_FILE_NOT_FOUND = 114
EXCEPTION_IO_DIRECTORY_NOT_FOUND = 115
EXCEPTION_NOT_UNIQUE = 120
EXCEPTION_DUPLICATE_KEY = 121
</pre>

<div id='id-Advanced'/>
###Advanced
<div id='id-Error-handling'/>
####Error handling
<div id='id-Kernel-Panic'/>
#####Kernel Panic
In order to catch kernel errors there is the function ````OnKernelPanic````. When this function gets called a kernel panic happened. A kernel panic is like a bluescreen in windows. When this function gets called the operating system is in an unstable state and stops its execution in order to prevent damage. Do not call any ArduinoOS functions inside this function. They will not work and their behavior is unpredictable. You can use this function for example to notify the user about the error code (LED, Serial, ...) or to reset the arduino.

Error codes:
````
KERNEL_ERROR_OUT_OF_STACK        = 1
KERNEL_ERROR_OUT_OF_MEMORY       = 2
KERNEL_ERROR_UNHANDLED_EXCEPTION = 3
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
<div id='id-Free-Memory'/>
#####Free Memory
In order to get the free memory use the function ````freeMemory````.
<div id='id-Free-Stack'/>
#####Free Stack
In order to get the free stack use the function ````freeStack````. Keep in mind that this function will require a bit of stack itself.
<div id='id-Stack'/>
####Stack
A Stack is a data type which allows putting (push) data on it and then take (pop) it from up to down. For more information see [Wikipedia](https://en.wikipedia.org/wiki/Stack_(abstract_data_type)). Various operations like calling functions require space on the stack. ArduinoOS will reserve stack space for every thread. In order to set the stack space use ```InitTaskWithStackSize``` or ```InitTaskWithStackSizeAndArgument``` when creating a new thread.

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
<div id='id-Thread-Arguments'/>
### Thread Arguments
In order to pass an argument to a thread use ````InitTaskWithArgument```` or ````InitTaskWithStackSizeAndArgument````. The passed argument must be of type void*.

Example:
```` c++
void mainThread()
{
	String *argument = new String("Hello World!");
	InitTaskWithArgument(thread2, argument);
}

void thread2(void* arg)
{
	String *argument = (String*)arg;
	delete argument;
}
````
<div id='id-Kernel-Tick-Period'/>
###Kernel Tick Period
The kernel tick period defines how many ticks should pass till a thread change gets initiated. 1000 ticks are 1 millisecond. So when the tick period is 1000 every millisecond a other thread gets executed. The default tick period is 2000. Keep in mind that [getPastMilliseconds](#id-Operating-System-uptime) will return a wrong value if you defined a tick period that is not divisible through 1000 (1 ms). This function is used by the kernel internal which also leads to inaccurate sleep periods. In order to set the tick period pass it as 3rd argument to ````KernelInitializer::InitializeKernel````.

Example:
```` c++
void setup()
{
	KernelInitializer::InitializeKernel(mainThread, STACK_SIZE_LARGE, 2000l);
}
````
<div id='id-Hardware-abstraction'/>
##Hardware abstraction
In order to make it easy to use different Hardware ArduinoOS offers different abstract classes for easy usage.
Supported hardware:
+ LED
+ RGB-LED
+ Keypad
+ Motor
+ Piezo speaker
+ Servo

Example:
```` c++
#include <RgbLed.h>

void mainThread()
{
	RgbLed led(10, 11, 12);   // Creates a new led with the pins 10 (red), 11 (green), 12 (blue)
	led.SetRGB(50, 100, 255);
	while (true)
	{
		led.TurnOn();
		sleep(5000);
		led.TurnOff();
		sleep(5000);
	}
}
````
