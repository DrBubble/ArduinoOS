#include "Kernel.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h> /* memset */
#define LOW(w) ((uint8_t) ((w) & 0xff))
#define HIGH(w) ((uint8_t) ((w) >> 8))
#define COMBINE(high, low) (uint16_t)((((uint16_t)high) << 8) | ((uint16_t)low));
#define	STACK_MEMORY_RANGE_IS_VALID(memoryRange) (memoryRange->start >= memoryRange->end)

#ifdef FEATURE_EXCEPTIONS
#define INVALID_EXCEPTION RAMEND
#endif

//uint8_t statusRegister;

#if defined FEATURE_ERROR_DETECTION
void(*OnKernelPanic)(uint8_t);
#endif

struct task *tasks;
struct task *currentTask;
struct task *previousTask;
struct task *lastTask;
struct task *idleTask;

const uint16_t STACK_SIZE_TINY = 64;
const uint16_t STACK_SIZE_SMALL = 96;
const uint16_t STACK_SIZE_MEDIUM = 128;
const uint16_t STACK_SIZE_LARGE = 256;
const uint16_t STACK_SIZE_GIANT = 512;
const uint16_t STACK_SIZE_DEFAULT = 128;

volatile uint16_t esp;

volatile unsigned long tickPeriods;
volatile unsigned long ticksExpired = 0;

volatile bool ignoreNextEip = false;

#if defined FEATURE_THREAD_ARGUMENTS
void *INVALID_ARGUMENT;
#endif

#if defined FEATURE_ERROR_DETECTION
#define STACK_CHECKING_NUMBER 174
#endif

void InitializeKernelState(unsigned long kernelTickPeriods)
{
	tickPeriods = kernelTickPeriods;
	tasks = NULL;
	currentTask = NULL;
	lastTask = NULL;
	previousTask = NULL;

#if FUNCTION_POINTER_SIZE == 3
	idleTask = InitTaskWithStackSizeAtomic(OS_IDLE, STACK_SIZE_SMALL);
#else
	idleTask = InitTaskWithStackSizeAtomic(OS_IDLE, STACK_SIZE_TINY);
#endif
	currentTask = idleTask;
#if defined FEATURE_THREAD_ARGUMENTS
	INVALID_ARGUMENT = malloc(sizeof(void *));
#endif
}

#define PUSH_CPU_STATE() \
	__asm__ __volatile__(\
		"push r0	\n"\
		"in r0, __SREG__	\n"\
		"push r0	\n"\
		"push r1	\n"\
		"push r2	\n"\
		"push r3	\n"\
		"push r4	\n"\
		"push r5	\n"\
		"push r6	\n"\
		"push r7	\n"\
		"push r8	\n"\
		"push r9	\n"\
		"push r10	\n"\
		"push r11	\n"\
		"push r12	\n"\
		"push r13	\n"\
		"push r14	\n"\
		"push r15	\n"\
		"push r16	\n"\
		"push r17	\n"\
		"push r18	\n"\
		"push r19	\n"\
		"push r20	\n"\
		"push r21	\n"\
		"push r22	\n"\
		"push r23	\n"\
		"push r24	\n"\
		"push r25	\n"\
		"push r26	\n"\
		"push r27	\n"\
		"push r28	\n"\
		"push r29	\n"\
		"push r30	\n"\
		"push r31	\n"\
	)

#define POP_CPU_STATE() \
	__asm__ __volatile__(\
		"pop r31	\n"\
		"pop r30	\n"\
		"pop r29	\n"\
		"pop r28	\n"\
		"pop r27	\n"\
		"pop r26	\n"\
		"pop r25	\n"\
		"pop r24	\n"\
		"pop r23	\n"\
		"pop r22	\n"\
		"pop r21	\n"\
		"pop r20	\n"\
		"pop r19	\n"\
		"pop r18	\n"\
		"pop r17	\n"\
		"pop r16	\n"\
		"pop r15	\n"\
		"pop r14	\n"\
		"pop r13	\n"\
		"pop r12	\n"\
		"pop r11	\n"\
		"pop r10	\n"\
		"pop r9 	\n"\
		"pop r8	    \n"\
		"pop r7	    \n"\
		"pop r6	    \n"\
		"pop r5	    \n"\
		"pop r4	    \n"\
		"pop r3 	\n"\
		"pop r2 	\n"\
		"pop r1	    \n"\
		"pop r0 	\n"\
		"out __SREG__, r0\n"\
		"pop r0	\n"\
	)

#define SAVE_CPU_STATE() \
	PUSH_CPU_STATE();\
	STORE_STACK_POINTER(esp)

#define RESTORE_CPU_STATE() \
	__asm__ __volatile__(\
		"out __SP_L__, %A0	\n"\
		"out __SP_H__, %B0	\n"\
		:: "r" (esp)\
	);\
	POP_CPU_STATE()

#define STORE_STACK_POINTER(target)\
	__asm__ __volatile__(\
		"in r24, __SP_L__	\n"\
		"in r25, __SP_H__	\n"\
		"sts " #target "+1, r25	\n"\
		"sts " #target ", r24	\n"\
		::: "r24", "r25"\
	)

#define SET_STACK_POINTER(stackPointer)\
	__asm__ __volatile__(\
		"OUT __SP_L__, %A0\n"\
		"OUT __SP_H__, %B0\n"\
		:: "r" (stackPointer)\
	)

#define INC_STACK() \
	__asm__ __volatile__(\
		"push r24"\
	)

#define DEC_STACK() \
	__asm__ __volatile__(\
		"pop r24"\
		::: "r24"\
	)

#define POP_STORE_8(target)\
	__asm__ __volatile__(\
		"pop r24		\n"\
		"sts " #target ", r24	\n"\
		::: "r24"\
	)

#define POP_STORE_16(target)\
	__asm__ __volatile__(\
		"pop r24		\n"\
		"sts " #target "+1, r24	\n"\
		"pop r24		\n"\
		"sts " #target ", r24	\n"\
		::: "r24"\
	)

#define POP_STORE_24(target)\
	__asm__ __volatile__(\
		"pop r24		\n"\
		"sts " #target "+2, r24	\n"\
		"pop r24		\n"\
		"sts " #target "+1, r24	\n"\
		"pop r24		\n"\
		"sts " #target ", r24	\n"\
		::: "r24"\
	)

#define PUSH_STORE_16(target)\
	__asm__ __volatile__(\
		"push r24		\n"\
		"push r25		\n"\
		:: "r" (target)\
	)


#if FUNCTION_POINTER_SIZE == 3
#define PUSH_FUNCTION_POINTER(functionPointer)\
	__asm__ __volatile__(\
		"mov r0, %A0	\n"\
		"push r0		\n"\
		"mov r0, %B0	\n"\
		"push r0		\n"\
		"mov r0, %C0	\n"\
		"push r0		\n"\
		:: "r" ((FUNCTION_POINTER)functionPointer)\
	)
#else
#define PUSH_FUNCTION_POINTER(functionPointer)\
	__asm__ __volatile__(\
		"mov r0, %A0	\n"\
		"push r0		\n"\
		"mov r0, %B0	\n"\
		"push r0		\n"\
		:: "r" ((FUNCTION_POINTER)functionPointer)\
	)
#endif


#if FUNCTION_POINTER_SIZE == 3
#define POP_FUNCTION_POINTER(functionPointer)\
	POP_STORE_24(functionPointer)
#else
#define POP_FUNCTION_POINTER(functionPointer)\
	POP_STORE_16(functionPointer)
#endif

#if FUNCTION_POINTER_SIZE == 3
#define STACK_REMOVE_FUNCTION_POINTER()\
	DEC_STACK();\
	DEC_STACK();\
	DEC_STACK()
#else
#define STACK_REMOVE_FUNCTION_POINTER()\
	DEC_STACK();\
	DEC_STACK()
#endif

#define InitContextSwitch()\
	__asm__ __volatile__(\
	"call switch_context"\
)

volatile bool ___calledFromInterrupt;

volatile void SwitchContextInternal()
{
	__asm__ __volatile__(
		"switch_context:"
	);

	cli();
	SAVE_CPU_STATE();

#if defined FEATURE_ERROR_DETECTION
	if (freeMemory() <= 30)
	{
		KernelPanic(KERNEL_ERROR_OUT_OF_MEMORY);
	}
#endif
	if (tasks != NULL)
	{
		if (___calledFromInterrupt)
		{
			ticksExpired += tickPeriods;
			//DecreaseTasksTime();
		}

		if (!ignoreNextEip)
		{
			if (currentTask != NULL)
			{
				//STORE_STACK_POINTER(esp);
				currentTask->esp = esp;

#if defined FEATURE_ERROR_DETECTION
				if (STACK_MEMORY_RANGE_IS_VALID(currentTask->memoryRange) && currentTask != idleTask)
				{
					if (esp > currentTask->memoryRange->start || esp < currentTask->memoryRange->end)
					{
						KernelPanic(KERNEL_ERROR_OUT_OF_STACK);
					}
					uint8_t *stackEnd = (uint8_t *)currentTask->memoryRange->end;
					if (*stackEnd != STACK_CHECKING_NUMBER)
					{
						KernelPanic(KERNEL_ERROR_OUT_OF_STACK);
					}
				}
#endif
			}
		}
		else
		{
			ignoreNextEip = false;
		}

		struct task *startTask = NULL;
		while (true)
		{
			if (currentTask == NULL || currentTask->next == NULL)
			{
				currentTask = tasks;
				previousTask = NULL;
			}
			else
			{
				previousTask = currentTask;
				currentTask = currentTask->next;
			}

			if (currentTask == idleTask && idleTask->next != NULL) {
				previousTask = currentTask;
				currentTask = idleTask->next;
			}

#if defined FEATURE_SLEEP
			if (currentTask->presumeOn <= ticksExpired) {
				currentTask->presumeOn = 0;
				break;
			}
#else
			break;
#endif

			if (currentTask == startTask)
			{
				currentTask = idleTask;
				previousTask = NULL;
				break;
			}

			if (startTask == NULL) {
				startTask = currentTask;
			}
		}


		if (currentTask->isFirstRun)
		{
			currentTask->isFirstRun = false;
			if (STACK_MEMORY_RANGE_IS_VALID(currentTask->memoryRange))
			{
				esp = currentTask->memoryRange->start;
				memset((void*)currentTask->memoryRange->end, 0, currentTask->memoryRange->start - currentTask->memoryRange->end);
			}
			else
			{
				/*if (lastTask != NULL && STACK_MEMORY_RANGE_IS_VALID(lastTask->memoryRange))
				{
					esp = lastTask->memoryRange->end - 1;
				}

				currentTask->memoryRange->end = esp - currentTask->memoryRange->start + 1;
				currentTask->memoryRange->start OnKernelPanic= esp;*/
				KernelPanic(12);
			}
			//Set Stack for new Task
			SET_STACK_POINTER(esp);
			currentTask->esp = esp;

#if defined FEATURE_ERROR_DETECTION
			uint8_t *stackEnd = (uint8_t *)currentTask->memoryRange->end;
			*stackEnd = STACK_CHECKING_NUMBER;
#endif
			//Push return address on Stack
			PUSH_FUNCTION_POINTER(TaskExecutionWrapper);
			asm("reti");
		}
		else
		{
			esp = currentTask->esp;

			//Restore Task
			RESTORE_CPU_STATE();
			asm("reti");
		}

	}

	KernelPanic(0); // Should never be reached
}


uint16_t currentEsp = 0;

static void TaskExecutionWrapper()
{
#if defined FEATURE_THREAD_ARGUMENTS
	if (currentTask->argument == INVALID_ARGUMENT)
	{
		void(*workerFunction)() = (void*)currentTask->workerFunction;
		workerFunction();
	}
	else
	{
		void(*workerFunction)(void *arg) = (void*)currentTask->workerFunction;
		workerFunction(currentTask->argument);
	}
#else
	currentTask->workerFunction();
#endif
	FinalizeCurrentTask();
	cli();
	___calledFromInterrupt = false;
	InitContextSwitch();
	OS_IDLE();
}

struct task *InitTask(void(*workerFunction)())
{
	return InitTaskWithStackSize(workerFunction, STACK_SIZE_DEFAULT);
}

struct task *InitTaskWithStackSize(void(*workerFunction)(), uint16_t stackSize)
{
	cli();
	struct task *createdTask = InitTaskWithStackSizeAtomic(workerFunction, stackSize);
	sei();
	cli();
	___calledFromInterrupt = false;
	InitContextSwitch();
	return createdTask;
}

struct task *InitTaskWithStackSizeAtomic(void(*workerFunction)(), uint16_t stackSize)
{
	struct task *task = malloc(sizeof(struct task));
	memset(task, 0, sizeof(struct task));
	task->workerFunction = (int)workerFunction;
	task->isFirstRun = true;
#if defined FEATURE_SLEEP
	task->presumeOn = 0;
#endif
	struct task** memoryInsertParentTask = malloc(sizeof(struct task*));
	struct task** memoryInsertNextTask = malloc(sizeof(struct task*));
	task->memoryRange = ReserveStackMemory(stackSize, memoryInsertParentTask, memoryInsertNextTask);

	if (*memoryInsertNextTask != NULL)
	{
		(*memoryInsertParentTask)->next = task;
		task->next = *memoryInsertNextTask;
	}
	else
	{
		if (lastTask != NULL)
		{
			lastTask->next = task;
		}

		if (tasks == NULL)
		{
			tasks = task;
		}

		lastTask = task;
	}
	free(memoryInsertParentTask);
	free(memoryInsertNextTask);

#if defined FEATURE_THREAD_ARGUMENTS
	task->argument = INVALID_ARGUMENT;
#endif

	return task;
}

#if defined FEATURE_THREAD_ARGUMENTS
struct task *InitTaskWithArgument(void(*workerFunction)(void *arg), void *arg)
{
	cli();
	struct task *createdTask = InitTaskWithStackSizeAtomic(workerFunction, STACK_SIZE_DEFAULT);
	createdTask->argument = arg;
	sei();
	sleep(0);
	return createdTask;
}

struct task *InitTaskWithStackSizeAndArgument(void(*workerFunction)(void *arg), uint16_t stackSize, void *arg)
{
	cli();
	struct task *createdTask = InitTaskWithStackSizeAtomic(workerFunction, stackSize);
	createdTask->argument = arg;
	sei();
	sleep(0);
	return createdTask;
}
#endif

//Function must be called in an atomic state
struct memory_range *ReserveStackMemory(uint16_t reservedStackSize, struct task** insertIntoParentTask, struct task** insertIntoNextTask)
{
	struct memory_range *memoryRange = malloc(sizeof(struct memory_range));
	memset(memoryRange, 0, sizeof(struct memory_range));
	memoryRange->start = RAMEND;
	memoryRange->end = memoryRange->start - reservedStackSize;
	*insertIntoNextTask = NULL;

	struct memory_range *previousValidMemoryRange = NULL;

	if (tasks != NULL)
	{
		struct task* currentTask = tasks;
		struct task* previousTask = NULL;
		while (currentTask != NULL)
		{
			if (STACK_MEMORY_RANGE_IS_VALID(currentTask->memoryRange))
			{
				if (previousValidMemoryRange != NULL && previousValidMemoryRange->end - currentTask->memoryRange->start >= reservedStackSize + 1)
				{
					memoryRange->start = previousValidMemoryRange->end - 1;
					memoryRange->end = memoryRange->start - reservedStackSize;
					*insertIntoParentTask = previousTask;
					*insertIntoNextTask = currentTask;
					return memoryRange;
				}
				previousValidMemoryRange = currentTask->memoryRange;
			}
			previousTask = currentTask;
			currentTask = currentTask->next;
		}
		if (lastTask != NULL && STACK_MEMORY_RANGE_IS_VALID(lastTask->memoryRange))
		{
			memoryRange->start = lastTask->memoryRange->end - 1;
			memoryRange->end = lastTask->memoryRange->end - reservedStackSize;
		}
	}

	return memoryRange;
}

void magicFunction() { }

void FinalizeCurrentTask()
{
	cli();
	FinalizeTask(currentTask, previousTask);
	sei();
}

volatile void OS_IDLE() {
	while (1);
}

void FinalizeTask(struct task* task, struct task *parentTask)
{
	ignoreNextEip = true;

	parentTask->next = task->next;
	if (lastTask == task)
	{
		lastTask = parentTask;
	}
	currentTask = parentTask;

	free(task->memoryRange);
	free(task);
}

void loop() {
}

#if defined FEATURE_SLEEP
void sleep(unsigned long milliseconds)
{
	sleepTicks(milliseconds * 1000);
}

void sleepTicks(unsigned long ticks)
{
	if (ticks == 0)
	{
		cli();
		___calledFromInterrupt = false;
		//while (currentTask->remainingIdleTime != 0);
		InitContextSwitch();
		sei();
	}
	else
	{
		cli();
		___calledFromInterrupt = false;
		if (ticks < tickPeriods)
		{
			currentTask->presumeOn = ticksExpired + tickPeriods;
		}
		else
		{
			currentTask->presumeOn = ticksExpired + ticks;
		}
		InitContextSwitch();
		sei();
		//while (currentTask->remainingIdleTime != 0);
	}
}
#endif

#if defined FEATURE_LOCK
struct lock *GetLockObject()
{
	struct lock *lockObject = malloc(sizeof(struct lock));
	memset(lockObject, 0, sizeof(struct lock));
	return lockObject;
}

void AquireLock(struct lock *lockObject)
{
	while (true)
	{
		cli();
		if (lockObject->lockCount == 0 || lockObject->lockingTask == currentTask)
		{
			lockObject->lockCount++;
			lockObject->lockingTask = currentTask;
			return;
		}
		else
		{
			___calledFromInterrupt = false;
			InitContextSwitch();
		}
		sei();
	}
}

void ReleaseLock(struct lock *lockObject)
{
	cli();
	if(lockObject->lockCount != 0 && lockObject->lockingTask == currentTask)
	{
		lockObject->lockCount--;
	}
	sei();
}
#endif

#if defined FEATURE_ERROR_DETECTION
static void KernelPanic(uint8_t errorCode)
{
	TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12)); //Detaches interrupt
	SET_STACK_POINTER(RAMEND); // Setting stack pointer to the end
	sei();
	if (OnKernelPanic != NULL)
	{
		OnKernelPanic(errorCode);
	}
	OS_IDLE();
}
#endif

int freeMemory()
{
	extern int __heap_start, *__brkval;
	if (STACK_MEMORY_RANGE_IS_VALID(lastTask->memoryRange))
	{
		return ((int)(lastTask->memoryRange->end)) - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
	}
	else
	{
		int v;
#pragma GCC diagnostic ignored "-Wreturn-local-addr"
		return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
	}
}

uint16_t freeStackStackPointer = 0;

int freeStack()
{
	if (!STACK_MEMORY_RANGE_IS_VALID(currentTask->memoryRange))
	{
		return UNKOWN_STACK_SIZE;
	}

	uint16_t stackPointer;
	cli();
	STORE_STACK_POINTER(freeStackStackPointer);
	stackPointer = freeStackStackPointer;
	sei();

	return stackPointer - currentTask->memoryRange->end;
}

long unsigned getElapsedMilliseconds()
{
	return ticksExpired / 1000;
}

long unsigned getElapsedTicks()
{
	return ticksExpired;
}

#ifdef FEATURE_EXCEPTIONS
volatile uint16_t exceptionThrowEsp;

void ThrowException(uint16_t exception)
{
	cli();
	if (currentTask->exceptionThrowEsp == 0)
	{
		KernelPanic(KERNEL_ERROR_UNHANDLED_EXCEPTION);
	}
	if (exception == 0)
	{
		exception = INVALID_EXCEPTION;
	}
	currentTask->exception = exception;

	// Remove return address from stack
	STACK_REMOVE_FUNCTION_POINTER();

	SET_STACK_POINTER(currentTask->exceptionThrowEsp);
	__asm__ __volatile__(
		"pop r24		\n"
		"sts exceptionThrowEsp+1, r24	\n"
		"pop r24		\n"
		"sts exceptionThrowEsp, r24	\n"
		::: "r24"
	);
	currentTask->exceptionThrowEsp = exceptionThrowEsp;

	POP_CPU_STATE();
	sei();
}

volatile FUNCTION_POINTER catchAddress;
void RegisterTry()
{
	cli();

	// Storing return address
#if FUNCTION_POINTER_SIZE == 3
	__asm__ __volatile__(
		"pop r24			\n"
		"pop r25			\n"
		"pop r26			\n"
		"sts catchAddress, r26		\n"
		"sts catchAddress+1, r25	\n"
		"sts catchAddress+2, r24	\n"
		"push r26			\n"
		"push r25			\n"
		"push r24			\n"
		::: "r24", "r25", "r26"
	);
#else
	__asm__ __volatile__(
		"pop r24			\n"
		"pop r25			\n"
		"sts catchAddress, r25		\n"
		"sts catchAddress+1, r24	\n"
		"push r25			\n"
		"push r24			\n"
		::: "r24", "r25"
	);
#endif

	// Saving cpu state
	PUSH_CPU_STATE();

	PUSH_STORE_16(currentTask->exceptionThrowEsp);
	STORE_STACK_POINTER(exceptionThrowEsp);
	currentTask->exceptionThrowEsp = exceptionThrowEsp;

	// Putting return address on stack
	PUSH_FUNCTION_POINTER(catchAddress);

	sei();
}

volatile FUNCTION_POINTER retAddress;
void ClearTry()
{
	cli();

	if (!HasException())
	{
		POP_FUNCTION_POINTER(retAddress);
		POP_STORE_16(exceptionThrowEsp);
		currentTask->exceptionThrowEsp = exceptionThrowEsp;

		POP_CPU_STATE();
		STACK_REMOVE_FUNCTION_POINTER();

		PUSH_FUNCTION_POINTER(retAddress);
	}
	currentTask->exception = 0;
	sei();
}

bool HasException()
{
	return currentTask->exception != 0;
}

uint16_t GetException()
{
	if (currentTask->exception == INVALID_EXCEPTION)
	{
		return 0;
	}
	return currentTask->exception;
}


bool IsBaseException(uint16_t baseException, uint16_t exception)
{
	if (baseException == EXCEPTION || baseException == exception)
	{
		return true;
	}
	if (baseException < 50 || baseException % 10 != 0)
	{
		return false;
	}
	if (baseException % 1000 == 0)
	{
		int16_t diff = exception - baseException;
		return diff >= 0 && diff < 1000;
	}
	else if (baseException % 100 == 0)
	{
		int16_t diff = exception - baseException;
		return diff >= 0 && diff < 100;
	}
	else
	{
		int16_t diff = exception - baseException;
		return diff >= 0 && diff < 10;
	}
}
#endif // FEATURE_EXCEPTIONS

void SwitchContext()
{
	cli();
	InitContextSwitch();
	sei();
}