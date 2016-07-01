# ArduinoOS
ArduinoOS is an operating system for arduino which supports multithreading and hardware abstraction. The kernel of ArduinoOS is entirely written in C and optimized for minimal memory usage.

Max Threads on device (1 System Thread):<br>
Arduino Uno: 20 Threads<br>
Arduino Mega: 90 Threads

## Table of Contents
1 [Setup](#id-Setup)<br>
2 [OS-Usage](#id-Usage)<br>
&nbsp;&nbsp;&nbsp;&nbsp;2.1 [Basics](#id-Basics)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.1 [Sleep](#id-Sleep)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.2 [Create Thread](#id-Create-Thread)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.3 [Operating System uptime](#id-Operating-System-uptime)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.4 [Locks](#id-Locks)<br>
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
<div id='id-Operating-System-uptime'/>
####Operating System uptime
To get the uptime of the operating system you can use ```getPastMilliseconds```.
<div id='id-Locks'/>
####Locks
In order to keep you application thread safe you can use locks. With locks you can prevent an other thread to access a variable, function, ... in an unsafe state.

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
<div id='id-Kernel-Panic'/>
#####Kernel Panic
In order to catch kernel errors there is the function ````OnKernelPanic````. When this function gets called a kernel panic happened. A kernel panic is like a bluescreen in windows. When this function gets called the operating system is in an unstable state and stops its execution in order to prevent damage. Do not call any ArduinoOS functions inside this function. They will not work and their behavior is unpredictable. You can use this function for example to notify the user about the error code (LED, Serial, ...) or to reset the arduino.

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
<div id='id-Free-Memory'/>
#####Free Memory
In order to require the free memory use the function ````freeMemory````.
<div id='id-Free-Stack'/>
#####Free Stack
In order to require the free stack use the function ````freeStack````. Keep in mind that this function will require a bit of stack itself.
<div id='id-Stack'/>
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
<div id='id-Thread-Arguments'/>
### Thread Arguments
In order to pass an argument to a thread use ````InitTaskWithArgument```` or ````InitTaskWithStackSizeAndArgument````. The passed argument must be an void*.

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
The kernel tick period defines how many ticks sould pass till a thread change gets initiated. 1000 ticks are 1 millisecond. So when the tick period is 1000 every millisecond a other thread gets executed. The default tick period is 2000. Keep in mind that [getPastMilliseconds](#id-Operating-System-uptime) will return a wrong value if you defined a tick period that is not divisible through 1000 (1 ms). This function is used by the kernel internal which also leads to inaccurate sleep periods. In order to set the tick period pass it as 3rd argument to ````KernelInitializer::InitializeKernel````.

Example:
```` c++
void setup()
{
	KernelInitializer::InitializeKernel(mainThread, STACK_SIZE_LARGE, 2000l);
}
````
<div id='id-Hardware-abstraction'/>
##Hardware abstraction
In order to make it easy to use different Hardware ArduinoOS offers different abstact classes for easy usage.
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
	}
}
````
