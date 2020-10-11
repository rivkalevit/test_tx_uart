//#include <QGraphicsScene>


#include "renderthread.h"
//#include "socket.h"
#include "uart.h"
//#define VID_HEADER_ID 0xA55A

#include <string.h>

#include <stdio.h>
#include <stdlib.h>



RenderThread::RenderThread(QObject *parent)
    : QThread(parent)
{
    abort = 0;
    connect_status = STATUS_DISCONNECTED;
    serial_port = new QSerialPort();
}

RenderThread::~RenderThread()
{
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    delete serial_port;
    wait();
}
/*#include <string.h>

#include <stdio.h>
#include <stdlib.h>*/
void RenderThread::run()
{
     UART_Handle_t h_serial = 0;
     int num = 1000;
     int ret = 0;
     h_serial = UART_Open(remote_port.toLatin1().data(),115200);
     if(!h_serial)
         return;
     char *cmdBuff = (char *)calloc(0x100,1);
     itoa(num, cmdBuff, 10);

     while(1){

        ret = UART_write_buffer_timeout(h_serial,(unsigned char*)cmdBuff,strlen(cmdBuff),0);
        //strcpy(cmdBuff2,cmdBuff);
        emit renderedValue(cmdBuff);

        msleep((unsigned long)interval);
        num++;
        itoa(num, cmdBuff, 10);
     }

#if 0

    int ret = 0;
    int num = 1000;
    QByteArray num_ba;
    num_ba.setNum(num);

    serial_port->setPortName(remote_port);
    serial_port->setBaudRate(QSerialPort::Baud115200);
   /* serial_port->setDataBits(QSerialPort::Data8);
    serial_port->setParity(QSerialPort::NoParity);
    serial_port->setStopBits(QSerialPort::OneStop);*/
    //serial_port->setFlowControl(QSerialPort::NoFlowControl);

    if(serial_port->open(QIODevice::ReadWrite)){
        connect_status = STATUS_CONNECTED;
    }
    else{
        connect_status = STATUS_FAILED;
        return;
    }

  //  serial_port->isOpen()

    //init port
    while(1){
        qint64 sret;
        if (abort)
           return;

        sret = serial_port->write(num_ba);
        serial_port->waitForBytesWritten(1000);
     //   mutex.lock();
        emit renderedValue(num_ba);
     //   mutex.unlock();

        num++;
        num_ba.setNum(num);

        msleep((unsigned long)interval);
    }
#endif

}

