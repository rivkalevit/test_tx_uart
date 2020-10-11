/****************************************************************************

File name   : oslite.h

Description : API of the actions of task.

*****************************************************************************/

#ifndef OSLITE
#define OSLITE

#ifdef __cplusplus
extern "C" {
#endif


#if defined (WIN32)
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define DLLEXPORT_API __declspec(dllexport)
#define DLLIMPORT_API __declspec(dllimport)

typedef HANDLE OS_Task_t;
typedef HANDLE OS_Semaphore_t;
#elif defined(LINUX)
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <semaphore.h>
 typedef pthread_t OS_Task_t;
 typedef unsigned long OS_Semaphore_t;
#endif


 #ifndef	dbg_assert
#ifdef DEBUG
#define dbg_assert(e) assert((e))
#else
#define dbg_assert(e)
#endif
#endif


 typedef  unsigned int     U32;


#define TIMEOUT_INFINITY  (U32)-1
#define TIMEOUT_IMMEDIATE  (U32)0


 #define OS_TIMEOUT -1
#define OS_SUCCESS 0

typedef void * (*OS_TaskFunc_t)(void *);


/**********************************************
                                semaphore and signals
************************************************/

int OS_SetSignal(OS_Semaphore_t Semaphore);
int OS_WaitSignal(OS_Semaphore_t Semaphore,OS_Semaphore_t lock,U32 Time);

int OS_WaitSemaphore(OS_Semaphore_t Semaphore);
int OS_SetSemaphore(OS_Semaphore_t Semaphore);
OS_Semaphore_t OS_AllocSemaphore(void);
int OS_DeleteSemaphore(OS_Semaphore_t Semaphore);
OS_Semaphore_t OS_AllocSignal(void);
int OS_DeleteSignal(OS_Semaphore_t Semaphore);
int OS_SetTaskPriority (OS_Task_t Task,int Priority);
//int OS_PartitionStatus(OS_Partition_t Partition, OS_PartitionStatus_t* Status);
int OS_GetSignalCount(OS_Semaphore_t Semaphore);




/**********************************************
                                Interrupt handling
************************************************/
void OS_MSleep(U32 ms);
/*int OS_InterruptInstall(int InterruptNumber,int InterruptLevel,ISRHandler_t ISR,void*pParam);
int OS_InterruptEnable(int InterruptNumber,int Enable);
// For Debug
void OS_InterruptShoot(int InterruptNumber);
void OS_InterruptSetMask(int InterruptNumber,U32 Mask);
U32 OS_InterruptGetEventFlag(int InterruptNumber);
void OS_InterruptSetEvent(int InterruptNumber , U32 Event);
void OS_InterruptClear(int InterruptNumber,U32 Mask);
// Milisecond sleep
void OS_MSleep(U32 ms);
// Micro second sleep
void OS_USleep(U64 ns);
U32 OS_GetKHClock(OS_GetKHClock);
int64_t OS_GetMHClock(void);
uint64_t OS_NTPtime64 (void);
U32 OS_GetChipID(void);*/

U32 OS_GetKHClock(void);

/**********************************************
                                task
************************************************/
OS_Task_t  OS_TaskCreate(U32 StackSize,char *TaskName, OS_TaskFunc_t ,int Priority,void *Param);
int OS_TaskWait(OS_Task_t Task,U32 TimeOut);
void OS_TaskDelete(OS_Task_t Task);
OS_Task_t  OS_TaskGetSelf(void);
int OS_IsTaskRunning(OS_Task_t Task);
int  OS_IsTaskEqual(OS_Task_t taskid1,OS_Task_t taskid2);

#ifdef __cplusplus
}
#endif


#endif


