#include "Kernel.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#define LOW(w) ((uint8_t) ((w) & 0xff))
#define HIGH(w) ((uint8_t) ((w) >> 8))
#define COMBINE(high, low) (uint16_t)((((uint16_t)high) << 8) | ((uint16_t)low));
#define	STACK_MEMORY_RANGE_IS_VALID(memoryRange) (memoryRange->start >= memoryRange->end)

uint8_t statusRegister;

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
volatile unsigned long pastMilliseconds = 0;

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

#if defined (__AVR_ATmega2560__)
	idleTask = InitTaskWithStackSizeAtomic(OS_IDLE, STACK_SIZE_SMALL);
#else
	idleTask = InitTaskWithStackSizeAtomic(OS_IDLE, STACK_SIZE_TINY);
#endif
	currentTask = idleTask;
#if defined FEATURE_THREAD_ARGUMENTS
	INVALID_ARGUMENT = malloc(sizeof(void *));
#endif
}


void SwitchContext(bool calledFromInterrupt)
{
	__asm__ __volatile__("cli\n mov r24, r28\n"
		"pop r28	\n"
		"pop r15	\n"
		"pop r14	\n"
		"pop r13	\n"
		"pop r12	\n"
		"pop r11	\n"
		"pop r10	\n"
		"pop r9	\n"
		"pop r8	\n"
		);

	//__asm__ __volatile__("cli\n mov r24, r17\n"
	//					 "pop r29	\n"
	//					 "pop r28	\n"
	//					 "pop r17	\n"
	//					 "pop r15	\n"
	//					 "pop r14	\n"
	//					 "pop r13	\n"
	//					 "pop r12	\n"
	//					 "pop r11	\n"
	//					 "pop r10	\n"
	//					 "pop r9	\n"
	//					 "pop r8	\n"
	//					 );

	__asm__ __volatile__(
		"push r0	\n"
		"in r0, __SREG__	\n"
		"cli		\n"
		"push r0	\n"
		"push r1	\n"
		"clr r1		\n"
		"push r2	\n"
		"push r3	\n"
		"push r4	\n"
		"push r5	\n"
		"push r6	\n"
		"push r7	\n"
		"push r8	\n"
		"push r9	\n"
		"push r10	\n"
		"push r11	\n"
		"push r12	\n"
		"push r13	\n"
		"push r14	\n"
		"push r15	\n"
		"push r16	\n"
		"push r17	\n"
		"push r18	\n"
		"push r19	\n"
		"push r20	\n"
		"push r21	\n"
		"push r22	\n"
		"push r23	\n"
		"push r24	\n"
		"push r25	\n"
		"push r26	\n"
		"push r27	\n"
		"push r28	\n"
		"push r29	\n"
		"push r30	\n"
		"push r31	\n"
		"in r26, __SP_L__	\n"
		"in r27, __SP_H__	\n"
		"sts esp+1, r27	\n"
		"sts esp, r26	\n"
		);

#if defined FEATURE_ERROR_DETECTION
	if (freeMemory() <= 20)
	{
		KernelPanic(KERNEL_ERROR_OUT_OF_MEMORY);
	}
#endif
	if (tasks != NULL)
	{
		statusRegister = SREG;

		if (calledFromInterrupt)
		{
			pastMilliseconds += tickPeriods / 1000;
		}
		if (!ignoreNextEip)
		{
			if (currentTask != NULL)
			{
				currentTask->esp = esp;

#if defined FEATURE_ERROR_DETECTION
				if (STACK_MEMORY_RANGE_IS_VALID(currentTask->memoryRange))
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
			if (currentTask->presumeOn <= pastMilliseconds) {
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
				if (lastTask != NULL && STACK_MEMORY_RANGE_IS_VALID(lastTask->memoryRange)) {
					esp = lastTask->memoryRange->end - 1;
				}

				currentTask->memoryRange->end = esp - currentTask->memoryRange->start + 1;
				currentTask->memoryRange->start = esp;

			}

			//Set Stack for new Task
			__asm__ __volatile__(
				"OUT __SP_L__, %A0\n"
				"OUT __SP_H__, %B0\n"
				:: "r" (esp)
				);
			currentTask->esp = esp;

#if defined FEATURE_ERROR_DETECTION
			uint8_t *stackEnd = (uint8_t *)currentTask->memoryRange->end;
			*stackEnd = STACK_CHECKING_NUMBER;
#endif

			//Push return address on Stack
#if defined (__AVR_ATmega2560__)
			__asm__ __volatile__(
				"mov r0, %A0	\n"
				"push r0		\n"
				"mov r0, %B0	\n"
				"push r0		\n"
				"mov r0, %C0	\n"
				"push r0		\n"
				:: "r" ((unsigned long)TaskExecutionWrapper)
				);
#else
			__asm__ __volatile__(
				"mov r0, %A0	\n"
				"push r0		\n"
				"mov r0, %B0	\n"
				"push r0		\n"
				:: "r" (TaskExecutionWrapper)
				);
#endif
		}
		else
		{
			esp = currentTask->esp;
			//Restore Task
			__asm__ __volatile__(\
				"out __SP_L__, %A0	\n"\
				"out __SP_H__, %B0	\n"\
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
				:: "r" (esp));
		}

	}
	else
	{
		if (calledFromInterrupt)
		{
			pastMilliseconds += tickPeriods / 1000;
		}
		__asm__ __volatile__(\
			"out __SP_L__, %A0	\n"\
			"out __SP_H__, %B0	\n"\
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
			:: "r" (esp));
	}
	sei();
	if (calledFromInterrupt)
	{
		asm("reti");
	}

	asm("ret");
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
	SwitchContext(false);
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
#if defined FEATURE_SLEEP
	sleep(0);
#else
	cli();
	SwitchContext(false);
#endif
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
	struct task** memoryInsertNextTask = malloc(sizeof(struct task*));;
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
	memoryRange->start = reservedStackSize;
	memoryRange->end = reservedStackSize + 1;
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

void OS_IDLE() {
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
	if (milliseconds == 0)
	{
		cli();
		SwitchContext(false);
	}
	else
	{
		//cli();
		currentTask->presumeOn = pastMilliseconds + milliseconds;
		SwitchContext(false);
		//while (currentTask->presumeOn != 0);
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
		if (!lockObject->isLocked)
		{
			lockObject->isLocked = true;
			return;
		}
		else
		{
			SwitchContext(false);
		}
		sei();
	}
}

void ReleaseLock(struct lock *lockObject)
{
	cli();
	lockObject->isLocked = false;
	sei();
}
#endif

#if defined FEATURE_ERROR_DETECTION
static void KernelPanic(uint8_t errorCode)
{
	TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12)); //Detaches interrupt
	struct task *nextTask = tasks;
	while (nextTask != NULL)
	{
		nextTask = nextTask->next;
		free(nextTask->memoryRange);
		free(nextTask);
	}
	sei();
	memset((void*)esp, 0, RAMEND - esp);
	__asm__ __volatile__(
		"OUT __SP_L__, %A0\n"
		"OUT __SP_H__, %B0\n"
		:: "r" (RAMEND)
		);
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
	__asm__ __volatile__(
		"in r26, __SP_L__	\n"
		"in r27, __SP_H__	\n"
		"sts freeStackStackPointer+1, r27	\n"
		"sts freeStackStackPointer, r26	\n"
		::: "r26", "r27");
	stackPointer = freeStackStackPointer;
	sei();

	//return stackPointer;
	return stackPointer - currentTask->memoryRange->end;
}

long unsigned getPastMilliseconds()
{
	return pastMilliseconds;
}