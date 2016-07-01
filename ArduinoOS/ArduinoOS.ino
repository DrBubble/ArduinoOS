/*
 Name:		ArduinoOS.ino
 Created:	7/1/2016 6:27:04 PM
 Author:	Manuel
*/

#include "KernelInitializer.h"

void setup()
{
	KernelInitializer::InitializeKernel(mainThread);
}

void mainThread()
{

}