#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QSerialPort>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    refresh_ports_list();
    QSerialPort *serial_port;
    qint64 sret;
#if 0
    serial_port = new QSerialPort();

    serial_port->setPortName("com6");
    serial_port->setBaudRate(QSerialPort::Baud115200);
    //serial_port->open(QIODevice::ReadWrite);


    if (!serial_port->open(QIODevice::ReadWrite)) {
        qDebug("open port failed");
    }
    sret = serial_port->write("rivka"/*num_ba*/);

    int x=0;
    x++;
#endif
    char time_st[100];
    sprintf(time_st,"%s  %s",__DATE__,__TIME__);
    ui->date_time_lbl->setText(time_st);
    connect(&thread, SIGNAL(renderedValue(char*)), this, SLOT(updateValue(char*)));
}

MainWindow::~MainWindow()
{
    if(thread.isRunning())
        thread.terminate();
    delete ui;
}


void MainWindow::updateValue(char *value_st){
    char value[20];
    strcpy(value,value_st);
    ui->tx_data->setText(value);
}

void MainWindow::refresh_ports_list(){
     ui->ports_list_cb->clear();
     foreach(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        ui->ports_list_cb->addItem(port.portName());
    }
}

void MainWindow::on_pushButton_clicked()
{
    refresh_ports_list();
}




void MainWindow::on_set_btn_clicked()
{
    if(thread.isRunning())
        thread.exit();
    thread.set_interval(ui->interval_le->text().toInt());
    thread.set_remote_port(ui->ports_list_cb->currentText());
    thread.start(QThread::HighPriority);
}
