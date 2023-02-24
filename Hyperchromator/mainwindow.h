#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "hyperchromator.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_openBut_clicked();
    void on_openShutterBut_clicked();
    void on_closeShutterBut_clicked();
    void on_closeBut_clicked();
    void on_resetBut_clicked();
    void on_setWLBut_clicked();
    void on_getWLBut_clicked();
    void on_readPosBut_clicked();
    void on_speedBut_clicked();
    void on_f1But_clicked();
    void on_f2But_clicked();
    void on_f3But_clicked();
    void on_f4But_clicked();
    void on_incBut_clicked();
    void on_decBut_clicked();
    void on_clearBut_clicked();

private:
    Hyperchromator *HC;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
