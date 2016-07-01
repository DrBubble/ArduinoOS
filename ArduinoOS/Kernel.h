/*
Memory usage:
No Features -> 1 Thread: 50 Byte + Stack size
All Features -> 1 Thread: 56 Byte + Stack size

Max Threads on device (1 System Thread):
Arduino Uno	 -> All Features: 20 Threads
No Features:  21 Threads

Arduino Mega -> All Features: 90 Threads
No Features:  97 Threads
*/
#include "stdint.h"
#include <stdbool.h>
#include "Features.h"

#if defined (__AVR_ATmega2560__)
#define FUNCTION_POINTER unsigned long
#else
#define FUNCTION_POINTER uint16_t
#endif

extern void(*OnKernelPanic)(uint8_t);

struct task {
	struct task			*next;
	struct memory_range	*memoryRange;
	bool				isFirstRun;
#if defined FEATURE_THREAD_ARGUMENTS
	void				*argument;
	FUNCTION_POINTER	workerFunction;
#else
	void(*workerFunction)();
#endif
	volatile uint16_t   esp;
#if defined FEATURE_SLEEP
	volatile unsigned long presumeOn;
#endif
};

struct memory_range {
	uint16_t			start;
	uint16_t			end;
};

#if defined FEATURE_LOCK
struct lock {
	bool isLocked;
};
#endif



void OS_IDLE();
void InitializeKernelState(unsigned long tickPeriods);
void SwitchContext(bool calledFromInterrupt);

#if defined FEATURE_ERROR_DETECTION
#pragma GCC diagnostic ignored "-Wunused-function"
static void KernelPanic(uint8_t errorCode);
#endif

//Function must be called in an atomic state
struct memory_range *ReserveStackMemory(uint16_t reservedStackSize, struct task** insertIntoParentTask, struct task** insertIntoNextTask);

void FinalizeCurrentTask();
void FinalizeTask(struct task* task, struct task *parentTask);
static void TaskExecutionWrapper();
struct task *InitTaskWithStackSizeAtomic(void(*workerFunction)(), uint16_t stackSize);


//User functions, Just dont touch the other ones
extern const uint16_t STACK_SIZE_TINY;
extern const uint16_t STACK_SIZE_SMALL;
extern const uint16_t STACK_SIZE_MEDIUM;
extern const uint16_t STACK_SIZE_LARGE;
extern const uint16_t STACK_SIZE_GIANT;
extern const uint16_t STACK_SIZE_DEFAULT;

#define KERNEL_ERROR_OUT_OF_STACK 1
#define KERNEL_ERROR_OUT_OF_MEMORY 2


#define UNKOWN_STACK_SIZE -1

struct task *InitTask(void(*workerFunction)());

static void CallKernelInterrupt();

#if defined FEATURE_THREAD_ARGUMENTS
struct task *InitTaskWithArgument(void(*workerFunction)(void *arg), void *arg);
#endif

struct task *InitTaskWithStackSize(void(*workerFunction)(), uint16_t stackSize);
#if defined FEATURE_THREAD_ARGUMENTS
struct task *InitTaskWithStackSizeAndArgument(void(*workerFunction)(void *arg), uint16_t stackSize, void *arg);
#endif
#if defined FEATURE_SLEEP
void sleep(unsigned long milliseconds);
#endif
#if defined FEATURE_LOCK
struct lock *GetLockObject();
void AquireLock(struct lock *lockObject);
void ReleaseLock(struct lock *lockObject);
#endif
int freeMemory();
int freeStack();
#if defined FEATURE_ATTACH_KERNE_INTERRUPT
void AllocateInterruptStack(uint8_t stackSize);
void FreeInterruptStack();
void AttachKernelInterrupt(void(*kernelInterrupt)());
#endif
long unsigned getPastMilliseconds();