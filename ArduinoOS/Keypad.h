#pragma once
#include "stdint.h"
#include "stdlib.h"
#include "KeypadBase.h"
#include "KernelInitializer.h"

struct keypadListenerEntry {
	struct keypadListenerEntry *next;
	struct keypadListenerEntry *previous;
	char						pressedKey;
	FUNCTION_POINTER			OnKeyPressed;
	bool						takesArgument;
};


class Keypad
{
private:
#if defined FEATURE_THREAD_ARGUMENTS
	static void startWatchKeysThread(void *keyPad);
#endif
	struct keypadListenerEntry *keyListeners;
	struct keypadListenerEntry *lastkeyListener;
#if defined FEATURE_LOCK
	lock *keyListenerLock;
#endif
	bool watchKeys;
	void fireKeyListeners(char key);
	void AddKeyListener(FUNCTION_POINTER onKeyPressed, char pressedKey, bool takesArgument);
	bool RemoveKeyListener(FUNCTION_POINTER onKeyPressed);
	KeypadBase *keypad;
public:
	Keypad(uint8_t rowPins[], uint8_t columnPins[], char* keymap, uint8_t rows, uint8_t columns);
	~Keypad();
	char GetKey();
	char WaitForKey();
	KeyState GetKeyState();
	bool KeyStateChanged();
	void SetKeyHoldTime(unsigned int time);
	void SetDebounceTime(unsigned int time);
	void AddKeyListener(void(*onKeyPressed)(char key));
	void AddKeyListener(void(*onKeyPressed)(char key), char pressedKey);
	void AddKeyListener(void(*onKeyPressed)());
	void AddKeyListener(void(*onKeyPressed)(), char pressedKey);
	bool RemoveKeyListener(void(*onKeyPressed)(char key));
	bool RemoveKeyListener(void(*onKeyPressed)());
	//Blocking function that watches if keys are getting pressed an calls the event listeners
	void WatchKeys();
#if defined FEATURE_THREAD_ARGUMENTS
	//Starts the WatchKeys function in a new Thread
	void StartWatchingKeys();
	void StartWatchingKeys(uint16_t stackSize);
#endif
	void StopWatchingKeys();

};

