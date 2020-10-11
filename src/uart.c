
//#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "oslite.h"
//#include <pthread.h>
//#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#ifndef WIN32
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#endif
#ifdef WIN32
#include <windows.h>
//#include <iostream>
#include <stdlib.h>
#include <string.h>

//using namespace std;

#endif

#define UART_INTERNAL_USE

#include "uart.h"
#define MIN(a,b) ((a)>(b)?(b):(a))



/***********************************
uart handler
************************************/
/*typedef struct{
    int         fd;
    char        *dev;
    int		speed;
}uartHandle_t;*/


#ifdef WIN32
typedef  HANDLE  UARTHANDLE_t;
#elif LINUX
typedef unsigned int UARTHANDLE_t;
//unsigned int uartHandle;
#endif

/*================= IsHaveChar()=================
 *Check if is there char to read
 *===============================================*/
static int IsHaveChar (UARTHANDLE_t handle){
#ifdef WIN32

    COMSTAT cs;
    unsigned long errmask = 0;

    if (!ClearCommError(handle, &errmask, &cs)) {
        OS_MSleep(10);
       return 0;
    }
    if(cs.cbInQue >0)
        return cs.cbInQue;

   // OS_MSleep(10);
    return 0;

#endif
   // uartHandle_t *phandle =(uartHandle_t *)handle;
#ifdef LINUX
    struct timeval tv;
    fd_set rfds;
    int nread=0;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    memset(&rfds,0,sizeof(rfds));
    //FD_ZERO(&rfds);
    if(handle ==0)
        return 0;

    if(handle){
        FD_SET(handle, &rfds);
        nread = select(handle + 1, &rfds, NULL, NULL, &tv);
    }
    return (nread !=0);
#endif
}


int UART_IsHaveChar(UART_Handle_t handle){
    uartHandle_t *pHandle = (uartHandle_t*)handle;
    int tmp = IsHaveChar(pHandle->fd);
    return tmp>0;
}


void* CyclicGetData(void* cycBuff){
uartHandle_t *pHandle =(uartHandle_t*)cycBuff;
   int ret = 0;
   int size = 0;
   
   pHandle->active_task = 1;
   while(pHandle->active_task){
        if(IsHaveChar(pHandle->fd)==0)
            continue;

        size = pHandle->buff_size - pHandle->in;
        //fprintf(stderr,"fd = %d \n\r",pHandle->fd);
        //fprintf(stderr,"size = %d \n\r",size);

        ret = UART_readBuff((UART_Handle_t*)pHandle,&pHandle->buff[pHandle ->in],size);
        if(ret >0){
            pHandle->in += ret;
        }
        OS_WaitSemaphore(pHandle ->lock);

        if(pHandle->in == pHandle->buff_size)
            pHandle->in = 0;

   //ret = OS_WaitSignal(pHandle->signal,pHandle->lock,TIMEOUT_INFINITY);
         pHandle->data_size +=ret;
        // fprintf(stderr,"data_size = %d  ret(%d)\n\r",pHandle->data_size,ret);
         OS_SetSemaphore(pHandle ->lock);
     }

}

UART_Handle_t UART_Open(char *device,int buadrate){
    //*retHandle = 0;
    uartHandle_t *pHandle = (uartHandle_t*)calloc(1,sizeof(uartHandle_t));


#ifdef WIN32
    DCB PortDCB;
	COMMTIMEOUTS CommTimeouts;
    int STOPBITS = ONESTOPBIT;
    // Open the serial port.
    pHandle->fd = CreateFileA(device,// Name of the port
                                            GENERIC_READ | GENERIC_WRITE,   // Access (read-write) mode
                                            0,
                                            NULL,
                                            OPEN_EXISTING,
                                            0,
                                            NULL);

    // If it fails to open the port, return 0.
    if (  pHandle->fd == INVALID_HANDLE_VALUE )
    {
            //We failed to open!
            return 0;
    }
    //Get the default port setting information.
    //GetCommState (handle, &PortDCB);
    // Change the settings.
    memset(&PortDCB,0,sizeof(PortDCB));
    PortDCB.DCBlength = sizeof(DCB);
    PortDCB.BaudRate = buadrate;              // BAUD Rate
    PortDCB.ByteSize = 8;           // Number of bits/byte, 5-8
    PortDCB.Parity = 0;              // 0-4=no,odd,even,mark,space
    PortDCB.StopBits = STOPBITS;          // StopBits
    PortDCB.fNull = 0;					  // Allow NULL Receive bytes
    PortDCB.fBinary = 1;


    // Re-configure the port with the new DCB structure.
    if (!SetCommState (pHandle->fd, &PortDCB))
    {
            // Could not create the read thread.
            CloseHandle(pHandle->fd);
            return 0;
    }

    // Retrieve the time-out parameters for all read and write operations
    // on the port.

    //GetCommTimeouts (pHandle->fd, &CommTimeouts);
    memset(&CommTimeouts, 0x00, sizeof(CommTimeouts));
    CommTimeouts.ReadIntervalTimeout = 100;
    CommTimeouts.ReadTotalTimeoutConstant = 1000;
    CommTimeouts.WriteTotalTimeoutConstant = 10000;

    // Set the time-out parameters for all read and write operations on the port.
#if 0
    if (!SetCommTimeouts (pHandle->fd, &CommTimeouts))
    {
            // Could not create the read thread.
            CloseHandle(pHandle->fd);
           return 0;
    }
#endif
    // Clear the port of any existing data.
    if(PurgeComm(pHandle->fd, PURGE_TXCLEAR | PURGE_RXCLEAR)==0)
    {
            CloseHandle(pHandle->fd);
            return 0;
    }
#endif

#ifdef LINUX

    struct termios options;

   pHandle->fd = open(device, O_RDWR | O_NOCTTY);
    memset(&options,0,sizeof(options));
    options.c_cflag = B115200 | CS8 |CREAD | CLOCAL ;
    options.c_oflag = 0;
    options.c_lflag = 0;
    options.c_cc[VMIN] = 0; 	 /* block untill n bytes are received */
    options.c_cc[VTIME] = 0;	 /* block untill a timer expires (n * 100 mSec.) */
    if(pHandle->fd <0){
        fprintf(stderr,"===========================open err \n\r");
        return 0;
    }
    ret = tcsetattr(pHandle->fd, TCSANOW, &options);


#endif

    pHandle->lock = OS_AllocSemaphore();
    pHandle->signal = OS_AllocSignal();
    pHandle->buff_size = 0x100000;
    pHandle ->buff = (unsigned char*) calloc(1,pHandle->buff_size);
    pHandle->in = 0;
    pHandle ->out = 0;

 //   pHandle->hTask = OS_TaskCreate(0x2000,"getBuffTask",CyclicGetData,0,pHandle);
    return  (UART_Handle_t)pHandle;


}


void UART_Close (UART_Handle_t handle){
	uartHandle_t *pHandle = (uartHandle_t*)handle;
    if(pHandle ==0)
        return;

    pHandle->active_task=0;
	if(pHandle->hTask)
		OS_TaskDelete(pHandle->hTask);
	
#ifdef WIN32>
    CloseHandle(pHandle->fd);

#endif
#ifdef LINUX
    if(pHandle->fd)
       close(pHandle->fd);


 #endif
 
 pHandle->fd =0;
         free((void*)handle);
    return;
}

int UART_is_not_empty (UART_Handle_t handle){
    int ret = 0;
	uartHandle_t *pHandle = (uartHandle_t*)handle;
    if(handle == 0)
        return 0;


        if(pHandle->data_size > 0)
            ret = 1;
        else
            OS_MSleep(10);

    return ret;

}


int UART_readBuff (UART_Handle_t handle,unsigned char* buff, int len){
	uartHandle_t *pHandle = (uartHandle_t*)handle;
	if(handle ==0)
		return 0;
	
#ifdef WIN32
    BOOL ret;
    unsigned long retlen = 0;

    ret = ReadFile(pHandle->fd,		// handle of file to read
                                    buff,		// pointer to buffer that receives data
                                    len,		// number of bytes to read
                                    &retlen,	// pointer to number of bytes read
                                    NULL		// pointer to structure for data
                                    );


    if(ret==0)
        return 0;
    if(retlen <=0){
        return 0;
    }

    return retlen;
    //return (int)ch;	//return the char

#endif
#ifdef LINUX
    int ret=0;
    if(pHandle->fd)
        ret = read(pHandle->fd,buff,len);
    return ret;
    #endif
}



int UART_readBuffTimeout(UART_Handle_t handle,unsigned char* buff, int len,int timeout){
    uartHandle_t *pHandle = (uartHandle_t*)handle;
    int rxsize = len;
    int start_time;
    int offset = 0;
    if(handle ==0)
        return 0;

    start_time = OS_GetKHClock();
#ifdef WIN32
    BOOL ret;
    unsigned long retlen = 0;

    while(rxsize>0){
    ret = ReadFile(pHandle->fd,		// handle of file to read
                   &buff[offset],		// pointer to buffer that receives data
                   len,		// number of bytes to read
                   &retlen,	// pointer to number of bytes read
                   NULL		// pointer to structure for data
                   );
    if(ret <= 0)
        return 0;
    rxsize -= retlen;
    offset += retlen;

    if(OS_GetKHClock() - start_time >= timeout)
        break;
    }

    if(retlen <= 0)
        return 0;
    return offset ;

#endif
#ifdef LINUX
    int ret=0;
    if(pHandle->fd)
        ret = read(pHandle->fd,buff,len);
    return ret;
    #endif
}

int  UART_read_buff_timeout(UART_Handle_t handle ,unsigned char* buff,int len, int timeoutms){
	
	uartHandle_t *pHandle = (uartHandle_t*)handle;
    unsigned int start_time = OS_GetKHClock();
    int offset=0;
    int rxsize = len;
    int cpysize = 0;

    while(rxsize>0){
        if(pHandle->data_size == 0){
            OS_MSleep(10);
            goto chktimeout;
        }
        cpysize = MIN(rxsize,pHandle->data_size);
        cpysize = MIN(cpysize,pHandle->buff_size-pHandle->out);
        memcpy(buff,&pHandle->buff[pHandle->out],cpysize);
        rxsize -= cpysize;

        pHandle->out = (pHandle->out + cpysize) % pHandle->buff_size;
        OS_WaitSemaphore(pHandle->lock);
        buff +=cpysize;
        offset +=cpysize;
        pHandle->data_size -=cpysize;
        OS_SetSemaphore(pHandle->lock);

chktimeout:
        if(OS_GetKHClock() - start_time > timeoutms)
            break;
    }
    return offset;

}



int UART_write_buffer_timeout(UART_Handle_t handle ,unsigned char* buff, int len,int timeoutms){
    uartHandle_t *pHandle = (uartHandle_t*)handle;
    int offset = 0;
    int txsize = len;
    if(!handle){
        fprintf(stderr,"%s.%d error write buffer\n",__FUNCTION__,__LINE__);
        return -1;
    }

#ifdef WIN32
    int ret=0;
    unsigned int StartTime = OS_GetKHClock();
    while(txsize>0){
        unsigned long retlen = 0;
        int cpbyt = txsize;
        retlen = 0;
        ret = WriteFile(pHandle->fd,			// Port handle
               &buff[offset],			// Pointer to the data to write
               cpbyt,			// Number of bytes to write
               &retlen,	// Pointer to the number of bytes written
               NULL);			// Must be NULL
        if(ret){
            txsize -=retlen;
            offset +=retlen;
            if(txsize > 0)
               OS_MSleep(1);
        }
        else{
            fprintf(stderr,"%s.%d\n",__FUNCTION__,__LINE__);
            OS_MSleep(10);
        }
        if((OS_GetKHClock() - StartTime) > timeoutms)
            break;
    }

#endif
#ifdef LINUX
    int ret=0;
    unsigned int StartTime = OS_GetKHClock();
   if(pHandle->fd==0)
       return 0;

    while(txsize>0){
        ret = write(pHandle->fd,&buff[offset],txsize);
        if(ret  < 0)
            break;
        txsize -= ret;
        offset +=ret;
        if((OS_GetKHClock() - StartTime) > timeoutms)
            break;
    }

 #endif

    return offset;
}



