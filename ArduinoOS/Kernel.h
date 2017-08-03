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
	#define FUNCTION_POINTER_SIZE 3
	#define FUNCTION_POINTER unsigned long
#else
	#define FUNCTION_POINTER_SIZE 2
	#define FUNCTION_POINTER uint16_t
#endif

// System functions, do not use without in depth knowledge
extern void(*OnKernelPanic)(uint8_t);
extern struct task *currentTask;
extern volatile bool ___calledFromInterrupt;

struct task {
	struct task			*next;			// 0-1
	struct memory_range	*memoryRange;	// 2-5
	bool				isFirstRun;		// 6
#ifdef FEATURE_THREAD_ARGUMENTS
	void				*argument;		// 7-8
	FUNCTION_POINTER	workerFunction;	// 8-9|8-10
#else
	void(*workerFunction)();
#endif
	volatile uint16_t   esp;			// 10-11
#ifdef FEATURE_SLEEP
	volatile unsigned long presumeOn;	// 12-14
#endif
#ifdef FEATURE_EXCEPTIONS
	volatile uint16_t exceptionThrowEsp; // 13-14
	volatile uint16_t exception;		 // 15-16
#endif
};

struct memory_range {
	uint16_t			start;
	uint16_t			end;
};

#if defined FEATURE_LOCK
struct lock {
	uint8_t lockCount;
	struct task *lockingTask;
};
#endif

void OS_IDLE();
void InitializeKernelState(unsigned long tickPeriods);

#ifdef FEATURE_ERROR_DETECTION
#pragma GCC diagnostic ignored "-Wunused-function"
static void KernelPanic(uint8_t errorCode);
#endif

//Function must be called in an atomic state
struct memory_range *ReserveStackMemory(uint16_t reservedStackSize, struct task** insertIntoParentTask, struct task** insertIntoNextTask);

void FinalizeCurrentTask();
void FinalizeTask(struct task* task, struct task *parentTask);
static void TaskExecutionWrapper();
struct task *InitTaskWithStackSizeAtomic(void(*workerFunction)(), uint16_t stackSize);

#ifdef FEATURE_EXCEPTIONS
void RegisterTry();
void ThrowException(uint16_t exception);
void ClearTry();
void __RETEX(uint16_t begin, uint8_t size);
#endif

//User functions
extern const uint16_t STACK_SIZE_TINY;
extern const uint16_t STACK_SIZE_SMALL;
extern const uint16_t STACK_SIZE_MEDIUM;
extern const uint16_t STACK_SIZE_LARGE;
extern const uint16_t STACK_SIZE_GIANT;
extern const uint16_t STACK_SIZE_DEFAULT;

#define KERNEL_ERROR_OUT_OF_STACK 1
#define KERNEL_ERROR_OUT_OF_MEMORY 2
#define KERNEL_ERROR_UNHANDLED_EXCEPTION 3

#define UNKOWN_STACK_SIZE -1

struct task *InitTask(void(*workerFunction)());

#ifdef FEATURE_THREAD_ARGUMENTS
struct task *InitTaskWithArgument(void(*workerFunction)(void *arg), void *arg);
#endif

struct task *InitTaskWithStackSize(void(*workerFunction)(), uint16_t stackSize);
#ifdef FEATURE_THREAD_ARGUMENTS
struct task *InitTaskWithStackSizeAndArgument(void(*workerFunction)(void *arg), uint16_t stackSize, void *arg);
#endif
void SwitchContext();
#ifdef FEATURE_SLEEP
void sleep(unsigned long milliseconds);
void sleepTicks(unsigned long ticks);
#endif
#ifdef FEATURE_LOCK
struct lock *GetLockObject();
void AquireLock(struct lock *lockObject);
void ReleaseLock(struct lock *lockObject);
#endif
int freeMemory();
int freeStack();
long unsigned getElapsedMilliseconds();
long unsigned getElapsedTicks();

#ifdef FEATURE_EXCEPTIONS
#define USER_EXCEPTIONS_BEGIN 1000

#define try RegisterTry(); if (!HasException()) {
#define catchType(baseException) } if (HasException()) { if (!IsBaseException(baseException, GetException())) rethrow();
#define catch } if (HasException()) {
#define clearException() } ClearTry();
#define throw(x) ThrowException(x);
#define rethrow() throw(GetException());
#define retex(x) auto __exception_retex_retValue_UNIQUE5928346827634206598 = x; ClearTry(); return __exception_retex_retValue_UNIQUE5928346827634206598;

bool HasException();
uint16_t GetException();

bool IsBaseException(uint16_t baseException, uint16_t exception);

#define EXCEPTION 0
#define EXCEPTION_OUT_OF_STACK 1
#define EXCEPTION_OUT_OF_MEMORY 2

#define EXCEPTION_NULL_POINTER 10
#define EXCEPTION_UNHANDLED_EXCEPTION 11
#define EXCEPTION_NOT_IMPLEMENTED 12
#define EXCEPTION_NOT_SUPORTED 13
#define EXCEPTION_INVALID_FORMAT 14
#define EXCEPTION_ACCES_VIOLATION 15
#define EXCEPTION_ACCES_TIMEOUT 16
#define EXCEPTION_INVALID_OPERATION_EXCEPTION 50
#define EXCEPTION_ARITHMETIC 60
#define EXCEPTION_ARITHMETIC_DIVIDE_BY_ZERO 61
#define EXCEPTION_BUFFER_OVERFLOW 70
#define EXCEPTION_INDEX_OUT_OF_RANGE 71
#define EXCEPTION_ASSERTION_ERROR 80
#define EXCEPTION_ILLEGAL_ARGUMENT 90
#define EXCEPTION_ILLEGAL_ARGUMENT_NULL 91
#define EXCEPTION_ILLEGAL_ARGUMENT_TO_BIG 92
#define EXCEPTION_ILLEGAL_ARGUMENT_TO_SMALL 93
#define EXCEPTION_ILLEGAL_ARGUMENT_OUT_OF_RANGE 94
#define EXCEPTION_ILLEGAL_ARGUMENT_FORMAT 95
#define EXCEPTION_IO 110
#define EXCEPTION_IO_EOF 111
#define EXCEPTION_IO_ACCESS_DENIED 112
#define EXCEPTION_IO_INVALID_DATA 113
#define EXCEPTION_IO_FILE_NOT_FOUND 114
#define EXCEPTION_IO_DIRECTORY_NOT_FOUND 115
#define EXCEPTION_NOT_UNIQUE 120
#define EXCEPTION_DUPLICATE_KEY 121
#endif

