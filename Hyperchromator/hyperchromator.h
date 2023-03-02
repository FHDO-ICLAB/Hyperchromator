#ifndef HYPERCHROMATOR_H
#define HYPERCHROMATOR_H

#include <QSerialPort>
#include <QString>
#include <QTimer>

class Hyperchromator: public QObject
{
    Q_OBJECT

public:
    Hyperchromator();
    void open(QString cal_path);
    void close();
    void openShutter();
    void closeShutter();
    void setWL(QString wl_input_str);
    void getWL();
    void readPos();
    void setSpeed(QString speed_str);
    void reset();
    void filter1();
    void filter2();
    void filter3();
    void filter4();
    void increment();
    void decrement();
    void clear();

private:
    QSerialPort serial_port;
    QByteArray  read_buf;
    unsigned char read_buf_ch[15];
    bool read2;
    bool read_pos;
    int num_bytes = 0;
    QTimer timer;
    float wl_buffer[100];
    float pos_buffer[100];
    int lines = 0;
    float wl_input = 0;

    float getPos(float wl);
    void setPos(float pos_str);

private slots:
    void read_serial();
    void updatePos();
    void initSpeed();
    void initWL();

};

#endif // HYPERCHROMATOR_H
