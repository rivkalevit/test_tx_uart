/****************************************************************************

File name   : oslite.c

Description : API of the actions of task.

*****************************************************************************/
#include "oslite.h"



#if defined LINUX
typedef struct
{
        pthread_mutex_t		Mutex;
        pthread_cond_t		Condition;
        int								SemCount;
}LinuxSemaphore_t;

#endif


int OS_GetTaskPriority (OS_Task_t Task,int *Priority){
#ifdef WIN32
                return GetThreadPriority((HANDLE)Task);
#endif

#ifdef LINUX
          struct sched_param   param;
          int				   policy;
          int				   rc;
          rc = pthread_getschedparam(Task, &policy, &param);
          *Priority = param.sched_priority;

          return rc;
#endif

}


int OS_WaitSemaphore(OS_Semaphore_t Semaphore){
#ifdef WIN32
        int err =	WaitForSingleObject((HANDLE)Semaphore,INFINITE);
        if(err == WAIT_OBJECT_0)
                return 0;
        return OS_TIMEOUT;

#endif

#if defined(LINUX)
        LinuxSemaphore_t  *pSem = (LinuxSemaphore_t  *)Semaphore;
        int rc;

        rc = pthread_mutex_lock(&pSem->Mutex);
        assert(rc ==0);
        return 0;

#endif
}

int OS_SetSemaphore(OS_Semaphore_t Semaphore){
#ifdef WIN32
        long prev;
        int err;
        err =ReleaseSemaphore((HANDLE)Semaphore,1,&prev);
        if(err ) return 0;
        return -1;
#endif
#if defined(TI_BIOS)
        SEM_post(Semaphore);
#endif

#if defined(LINUX)
         LinuxSemaphore_t  *pSem = (LinuxSemaphore_t  *)Semaphore;
          int rc;
          rc = pthread_mutex_unlock(&pSem->Mutex);
          assert(rc==0);
#endif

  return 0;
}

OS_Semaphore_t OS_AllocSemaphore(void){
#ifdef WIN32
        return (OS_Semaphore_t)CreateSemaphore(0,1,100,0);
#endif
#if defined(TI_BIOS)
        return SEM_create(1,0);
#endif

#if defined(LINUX)
        int rc;
        LinuxSemaphore_t  *pSem;
        pSem     = (LinuxSemaphore_t *)calloc(1,sizeof(LinuxSemaphore_t));
        //memset((void*)pSem,0,sizeof(*pSem));
        rc = pthread_mutex_init(&pSem->Mutex, NULL);
        assert(rc==0);
        return (OS_Semaphore_t)pSem;
#endif
}

int OS_DeleteSemaphore(OS_Semaphore_t Semaphore){
#ifdef WIN32
        if(Semaphore==0) return -1;
        CloseHandle((HANDLE)Semaphore);
        return 0;
#endif
#if defined(TI_BIOS)
        SEM_delete(Semaphore);
        return 0;
#endif

#if defined(LINUX)
         LinuxSemaphore_t  *pSem = (LinuxSemaphore_t  *)Semaphore;
         pthread_mutex_destroy(&pSem->Mutex);
         free((void*)pSem);
         return 0;
#endif

}

void OS_MSleep(U32 ms){
#if defined(WIN32)
        Sleep(ms);
#endif
#if defined(TI_BIOS)
        TSK_sleep(ms );
#endif
#if defined(LINUX)
struct timespec timeOut,remains;
        timeOut.tv_sec = ms/1000;
        timeOut.tv_nsec = (ms % 1000) * 1000*1000;
        nanosleep(&timeOut, &remains);
#endif
}

U32 OS_GetKHClock(void){
#ifdef WIN32
        return clock();
#endif
#if defined(TI_BIOS)
        return CLK_getltime();
#endif
#if defined(FREESCALE_NETCOMM)
 return XX_GetClock();
#endif
#if defined(LINUX)
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec/1000;
#endif
}

int OS_SetSignal(OS_Semaphore_t Semaphore){
#ifdef WIN32
        long prev;
        int err;
        err = ReleaseSemaphore((HANDLE)Semaphore,1,&prev);

        if(err ) return 0;
        dbg_assert(0);

#endif
#if  defined(TI_BIOS)
                SEM_post(Semaphore);
#endif
#if defined(LINUX)
  LinuxSemaphore_t  *pSem = (LinuxSemaphore_t  *)Semaphore;
  pSem->SemCount++;
  pthread_cond_signal	(&pSem->Condition	);
#endif

        return 0;
}

int OS_WaitSignal(OS_Semaphore_t cond,OS_Semaphore_t lock,U32 Time ){

#ifdef WIN32

        int err=0;
        OS_SetSemaphore(lock);
        if(cond ==0) return -1;
        if(Time == TIMEOUT_INFINITY)
                err = WaitForSingleObject((HANDLE)cond,INFINITE);
        else{
                err = WaitForSingleObject((HANDLE)cond,Time);
        }
        if(err == WAIT_OBJECT_0){
                OS_WaitSemaphore(lock);
                return 0;
        }
        OS_WaitSemaphore(lock);
        return OS_TIMEOUT;
#endif

#if defined(LINUX)
        LinuxSemaphore_t  *pCond = (LinuxSemaphore_t  *)cond;
        LinuxSemaphore_t  *pLoc = (LinuxSemaphore_t  *)lock;
        int rc;
        int ret =0;

//        pthread_mutex_unlock(&pLoc->Mutex		);
        if(Time == TIMEOUT_INFINITY){
                rc = pthread_cond_wait(&pCond->Condition, &pLoc->Mutex);
        }
        else{
                struct timespec timeout={0};
                unsigned int StartTime;
                StartTime       = OS_GetKHClock() + Time;
                timeout.tv_sec	= StartTime/1000;
                timeout.tv_nsec	= (StartTime % 1000) * 1000 ;
                rc =  pthread_cond_timedwait(&pCond->Condition,  &pLoc->Mutex, &timeout);
                if(rc== ETIMEDOUT)
                        ret= OS_TIMEOUT;
        }

  //      pthread_mutex_lock	(&pLoc->Mutex);
        return ret;
#endif
}

OS_Semaphore_t OS_AllocSignal(void){
#ifdef WIN32
        return CreateSemaphore(0,0,100,0);
#endif
#if defined(TI_BIOS)
        return SEM_create(0,0);
#endif
#if defined(LINUX)
        int rc=0;
        LinuxSemaphore_t  *pSem;
        pSem     = (LinuxSemaphore_t  *)calloc(1,sizeof(LinuxSemaphore_t));
        assert(pSem !=0);
        memset((void*)pSem,0,sizeof(LinuxSemaphore_t));
        rc = pthread_cond_init(&pSem->Condition, NULL);
        assert(rc ==0);
        return (OS_Semaphore_t)pSem;
#endif
}

int OS_DeleteSignal(OS_Semaphore_t Semaphore){
#ifdef WIN32
        CloseHandle((HANDLE)Semaphore);
        return 0;
#endif

#if defined(TI_BIOS)
        SEM_delete(Semaphore);
        return 0;
#endif
#if defined(LINUX)
         LinuxSemaphore_t  *pSem = (LinuxSemaphore_t  *)Semaphore;
         pthread_cond_destroy(&pSem->Condition);
         free((void*)pSem);
         return 0;
#endif
}

OS_Task_t  OS_TaskCreate(U32 StackSize,char *TaskName,OS_TaskFunc_t TaskFunc,int Priority,void *Param){
#if  defined(WIN32)
        U32 ThreadId;
        OS_Task_t task ;
    //*task = _beginthread( function, stack_size, 0);
        task = (OS_Task_t)CreateThread(
                0,										// pointer to security attributes
                StackSize,								// initial thread stack size
                (LPTHREAD_START_ROUTINE) TaskFunc,       // pointer to thread function
                Param,									 // argument for new thread
                CREATE_SUSPENDED,									 // creation flags
                (unsigned long *)&ThreadId                                // pointer to receive thread ID
                );

        SetThreadPriority((HANDLE)task,Priority);
        ResumeThread((void*)task);
        return task;
#endif
#if defined(LINUX)
        pthread_t hthread;
        int rc;
        pthread_attr_t attr;
        rc = pthread_attr_init(&attr);
        rc = pthread_attr_setstacksize(&attr, StackSize);
//    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        rc = pthread_create(&hthread, &attr, (void*(*)(void*))TaskFunc, Param);
        if(rc != 0){
                return 0;
        }
    return (OS_Task_t )hthread;
#endif
}

int OS_TaskWait(OS_Task_t Task,U32 TimeOut){
#ifdef WIN32
U32 Time =TimeOut;
U32  terminat;

        while(GetExitCodeThread((HANDLE)Task,(unsigned long *)&terminat)){

                if((terminat != STILL_ACTIVE) )
                        return 0;

                if(TIMEOUT_INFINITY != TimeOut)
                {
                        Time-=1;
                        if(Time<1)
                                break;
                }
                if(TIMEOUT_IMMEDIATE == TimeOut)
                        break;
                OS_MSleep(1);
        }

#endif

#if defined(LINUX)
        U32 Time =TimeOut;
        while(OS_IsTaskRunning(Task)){
                if(TIMEOUT_INFINITY != TimeOut)
                {
                        Time-=10;
                        if(Time<10)
                                break;
                }
                if(TIMEOUT_IMMEDIATE == TimeOut)
                        break;
                OS_MSleep(1);
        }

#endif
        return -1;
}

int OS_IsTaskRunning(OS_Task_t Task){
#if  defined(WIN32)
        if(GetThreadPriority((HANDLE)Task) != THREAD_PRIORITY_ERROR_RETURN)
                return 1;
#endif

#if defined(LINUX)
        if(pthread_kill(Task, 0) == 0) return 1;
#endif
        return 0;
}

void OS_TaskDelete(OS_Task_t Task){
#ifdef WIN32
        if(Task){
                OS_TaskWait(Task,  TIMEOUT_INFINITY);
                CloseHandle((HANDLE)Task);
        }
#endif

#if defined(LINUX)
        int rc;

        //OS_TaskWait(Task,  TIMEOUT_INFINITY);
        rc = pthread_join(Task,0);
        rc = pthread_detach(Task);
#endif
}

OS_Task_t  OS_TaskGetSelf(void){
#if defined(WIN32)
        return (OS_Task_t)GetCurrentThread();
#endif
#if defined(TI_BIOS)
        OS_Task_t curtask;
        curtask = TSK_self();
        return curtask;
#endif
#if defined(LINUX)
        return pthread_self();
#endif

}




