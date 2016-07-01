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
