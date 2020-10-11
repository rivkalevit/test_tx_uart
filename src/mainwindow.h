#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "renderthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void refresh_ports_list();

private slots:
    void updateValue(char *val_st);

    void on_pushButton_clicked();



    void on_set_btn_clicked();

private:
    Ui::MainWindow *ui;
    RenderThread thread;
};

#endif // MAINWINDOW_H
