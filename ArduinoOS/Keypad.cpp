#include "Keypad.h"
#include "KernelInitializer.h"

Keypad::Keypad(uint8_t rowPins[], uint8_t columnPins[], char* keymap, uint8_t rows, uint8_t columns)
{
	keypad = new KeypadBase(keymap, rowPins, columnPins, rows, columns);
	keyListeners = NULL;

#if defined FEATURE_LOCK
	keyListenerLock = GetLockObject();
#endif
	watchKeys = false;
}

Keypad::~Keypad()
{
	delete keypad;

	struct keypadListenerEntry *keyListener = keyListeners;
	while (keyListener != NULL)
	{
		struct keypadListenerEntry *nextKeyListener = keyListener->next;
		delete keyListener;
		keyListener = nextKeyListener;
	}

#if defined FEATURE_LOCK
	free(keyListenerLock);
#endif
}

char Keypad::GetKey()
{
	return keypad->getKey();
}

char Keypad::WaitForKey()
{
	char key = NO_KEY;

	while (key == NO_KEY)
	{
		key = GetKey();
#if defined FEATURE_SLEEP
		sleep(0);
#endif
	}

	return key;
}

KeyState Keypad::GetKeyState()
{
	return keypad->getState();
}

bool Keypad::KeyStateChanged()
{
	return keypad->keyStateChanged();
}

void Keypad::SetKeyHoldTime(unsigned int time)
{
	keypad->setHoldTime(time);
}

void Keypad::SetDebounceTime(unsigned int time)
{
	keypad->setDebounceTime(time);
}

void Keypad::AddKeyListener(void(*onKeyPressed)(char key))
{
	AddKeyListener((FUNCTION_POINTER)onKeyPressed, NO_KEY, true);
}

void Keypad::AddKeyListener(void(*onKeyPressed)(char key), char pressedKey)
{
	AddKeyListener((FUNCTION_POINTER)onKeyPressed, pressedKey, true);
}


void Keypad::AddKeyListener(void(*onKeyPressed)())
{
	AddKeyListener((FUNCTION_POINTER)onKeyPressed, NO_KEY, false);
}

void Keypad::AddKeyListener(void(*onKeyPressed)(), char pressedKey)
{
	AddKeyListener((FUNCTION_POINTER)onKeyPressed, pressedKey, false);
}

void Keypad::AddKeyListener(FUNCTION_POINTER onKeyPressed, char pressedKey, bool takesArgument)
{
#if defined FEATURE_LOCK
	AquireLock(keyListenerLock);
#endif
	if (keyListeners == NULL)
	{
		keyListeners = new keypadListenerEntry();
		keyListeners->pressedKey = pressedKey;
		keyListeners->next = NULL;
		keyListeners->previous = NULL;
		keyListeners->OnKeyPressed = onKeyPressed;
		keyListeners->takesArgument = takesArgument;
		lastkeyListener = keyListeners;
	}
	else
	{
		struct keypadListenerEntry *newKeyListener = new keypadListenerEntry();
		newKeyListener->pressedKey = pressedKey;
		newKeyListener->next = NULL;
		newKeyListener->previous = lastkeyListener;
		newKeyListener->OnKeyPressed = onKeyPressed;
		newKeyListener->takesArgument = takesArgument;
		lastkeyListener->next = newKeyListener;
		lastkeyListener = newKeyListener;
	}
#if defined FEATURE_LOCK
	ReleaseLock(keyListenerLock);
#endif
}

bool Keypad::RemoveKeyListener(void(*onKeyPressed)(char key))
{
	return RemoveKeyListener((FUNCTION_POINTER)onKeyPressed);
}

bool Keypad::RemoveKeyListener(void(*onKeyPressed)())
{
	return RemoveKeyListener((FUNCTION_POINTER)onKeyPressed);
}

bool Keypad::RemoveKeyListener(FUNCTION_POINTER onKeyPressed)
{
#if defined FEATURE_LOCK
	AquireLock(keyListenerLock);
#endif
	struct keypadListenerEntry *keyListener = keyListeners;
	while (keyListener != NULL)
	{
		if (keyListener->OnKeyPressed == onKeyPressed)
		{
			if (keyListener != keyListeners)
			{
				keyListener->previous->next = keyListener->next;
				delete keyListener;
			}
			else
			{
				keyListeners = keyListeners->next;
				delete keyListener;
			}
#if defined FEATURE_LOCK
			ReleaseLock(keyListenerLock);
#endif
			return true;
		}
		keyListener = keyListener->next;
	}

#if defined FEATURE_LOCK
	ReleaseLock(keyListenerLock);
#endif
	return false;
}

void Keypad::WatchKeys()
{
	watchKeys = true;
	while (watchKeys)
	{
#if defined FEATURE_LOCK
		AquireLock(keyListenerLock);
#endif
		char pressedKey = keypad->getKey();
		if (pressedKey != NO_KEY)
		{
			fireKeyListeners(pressedKey);
		}
#if defined FEATURE_LOCK
		ReleaseLock(keyListenerLock);
#endif
#if defined FEATURE_SLEEP
		sleep(10);
#endif
	}
}

void Keypad::StopWatchingKeys()
{
	watchKeys = false;
}

void Keypad::fireKeyListeners(char key)
{
	struct keypadListenerEntry *keyListener = keyListeners;
	while (keyListener != NULL)
	{
		if (keyListener->pressedKey == NO_KEY || key == keyListener->pressedKey)
		{
			FUNCTION_POINTER onKeyPressed = keyListener->OnKeyPressed;
			if (keyListener->takesArgument)
			{
				typedef void(*fptr)(char key);
				fptr function = (fptr)onKeyPressed;
				function(key);

			}
			else
			{

				typedef void(*fptr)();
				fptr function = (fptr)onKeyPressed;
				function();
			}
		}
		keyListener = keyListener->next;
	}
}

#if defined FEATURE_THREAD_ARGUMENTS
void Keypad::StartWatchingKeys()
{
	StartWatchingKeys(STACK_SIZE_DEFAULT);
}

void Keypad::StartWatchingKeys(uint16_t stackSize)
{
	InitTaskWithStackSizeAndArgument(startWatchKeysThread, stackSize, this);
}

void Keypad::startWatchKeysThread(void *keyPad)
{
	((Keypad *)keyPad)->WatchKeys();
}
#endif