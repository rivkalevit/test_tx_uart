#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include <QSize>
#include <QWaitCondition>
#include <QSerialPort>


enum{
    STATUS_DISCONNECTED,
    STATUS_CONNECTED,
    STATUS_FAILED
};


class RenderThread : public QThread
{
    Q_OBJECT
    public:

    RenderThread(QObject *parent = 0);
    ~RenderThread();

public:
    void set_interval(int value){interval = value;}
    void set_remote_port(QString port){remote_port = port;}
    int get_connection_status(){return connect_status;}

    signals:
    void renderedValue(/*const QByteArray &value_st*/char *value_st);
    protected:
    void run() override;
    QMutex mutex;
    QWaitCondition condition;
    QSize resultSize;
    bool restart;
    bool abort;
    private:
    QString remote_port;
    QSerialPort *serial_port;
    int interval;
    int connect_status;

};

#endif // RENDERTHREAD_H
