#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hyperchromator.h"

#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    HC = new Hyperchromator();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_openBut_clicked(){
    HC->open();
}

void MainWindow::on_openShutterBut_clicked(){
    HC->openShutter();
}

void MainWindow::on_closeShutterBut_clicked(){
    HC->closeShutter();
}

void MainWindow::on_closeBut_clicked(){
    HC->close();
}

void MainWindow::on_resetBut_clicked(){
    HC->reset();
}

void MainWindow::on_setWLBut_clicked(){
    HC->setWL(ui->wlLine->text());
}

void MainWindow::on_getWLBut_clicked(){
    HC->getWL();
}

void MainWindow::on_readPosBut_clicked(){
    HC->readPos();
}

void MainWindow::on_speedBut_clicked(){
    HC->setSpeed(ui->speedLine->text());
}
void MainWindow::on_f1But_clicked(){
    HC->filter1();
}

void MainWindow::on_f2But_clicked(){
    HC->filter2();
}

void MainWindow::on_f3But_clicked(){
    HC->filter3();
}

void MainWindow::on_f4But_clicked(){
    HC->filter4();
}

void MainWindow::on_incBut_clicked(){
    HC->increment();
}

void MainWindow::on_decBut_clicked(){
    HC->decrement();
}

void MainWindow::on_clearBut_clicked(){
    HC->clear();
}
