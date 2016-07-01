# ArduinoOS
O
ArduinoOS is an operating system for arduino which supports multithreading and hardware abstaction.
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

##Usage
###Basics
####Sleep
In order to pause your programm you can use:
```c++
sleep(milliseconds);
```
Do not use ```delay``` since it does not allow the operating system to execute other tasks in the meanwhile and will block the thread for that time. That means ```delay(500)``` will block for 1 second if 2 Threads are running and for 2 seconds if 4 threads are running.
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
