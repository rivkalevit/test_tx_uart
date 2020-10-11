#ifndef UART_H
#define UART_H
#include "oslite.h"
#ifdef WIN32
#include <windows.h>
#include <commctrl.h>
#endif


typedef unsigned int UART_Handle_t;

typedef struct{
    char *dev;
    int speed;
    OS_Semaphore_t lock;
    OS_Semaphore_t signal;
    OS_Task_t get_buff_task;
    int in;
    int out;
    int buff_size;
    int data_size;
    unsigned char *buff;
    OS_Task_t hTask;
    HANDLE fd;
    int active_task;
}uartHandle_t;

/*
typedef struct {
    HANDLE hSerial;
    int m_baud;
    char m_port[40];
}uart_hanadle_t;
*/
//=================Functions==================


#ifdef __cplusplus
extern "C" {
#endif
UART_Handle_t  UART_Open(char *dev, int buadrate);
void UART_Close(UART_Handle_t handle);
int  UART_is_not_empty(UART_Handle_t handle);
int UART_readBuff (UART_Handle_t handle,unsigned char* buff, int len);
//int  UART_readBuff  (UARTHANDLE_t retHandle ,char* buffer,int length);
//int  UART_writeBuffer(UARTHANDLE_t retHandle,unsigned char* buffer,int length);
int  UART_read_buff_timeout(UART_Handle_t handle ,unsigned char* buffer,int length, int timeoutMS);
int  UART_write_buffer_timeout(UART_Handle_t handle ,unsigned char* buffer,int length, int timeoutMS);
int UART_readBuff (UART_Handle_t handle,unsigned char* buff, int len);
int UART_readBuffTimeout(UART_Handle_t handle,unsigned char* buff, int len,int timeoutMS);
int UART_IsHaveChar(UART_Handle_t handle);


#ifdef __cplusplus
}
#endif
#endif // UART_H
