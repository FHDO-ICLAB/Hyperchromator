/*Notes:
 * config.txt file shows max speed as 250.
 * open() resets driver and after 10 secs sets speed to max (250).
 * reset() resets speed to low value, so it is recommended to set speed after some seconds everytime the function is called.
 * calibration file must be added to build folder to be read
 * Institution: FH Dortmund
 * Author: Saul García
*/

#include "hyperchromator.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QByteArray>
#include <QString>
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <stdio.h>
#include <math.h>

using namespace std;

Hyperchromator::Hyperchromator(){

    read2 = false;
    read_pos = false;
    connect(&serial_port, SIGNAL(readyRead()), this, SLOT(read_serial()));
}

void Hyperchromator::read_serial(){

    QByteArray  read_buf_2;
    int num_bytes_2;
    long        pos_raw;
    float       pos;

    if(read2 == false){
        read_buf = serial_port.read(15);       //QByteArray readarray mit den empfangenen Daten füllen
        num_bytes = read_buf.size();
        memcpy(read_buf_ch,read_buf.constData(),num_bytes);
        //qDebug() << "Buf 1 " << read_buf.toHex();
    }
    // In case we have not received complete message
    else if(read2){
        read_buf_2 = serial_port.read(15);
        num_bytes_2 = read_buf_2.size();
        unsigned char read_buf_2_ch[num_bytes_2];
        memcpy(read_buf_2_ch,read_buf_2.constData(),num_bytes_2);
        //qDebug() << "Buf 2 " << read_buf_2.toHex();
        for(int i=0; i < num_bytes_2; i++){ read_buf_ch[i+num_bytes] = read_buf_2_ch[i]; }
        num_bytes += num_bytes_2;
    }

    if(read_pos == true && num_bytes == 15){
        // Read Position of motor
        pos_raw = (read_buf_ch[12] <<  24) | (read_buf_ch[13] << 16) | (read_buf_ch[10] << 8) | read_buf_ch[11];
        pos = stof(to_string(pos_raw));
        qDebug() << "Position: " << pos;
        read2 = false;
        read_pos = false;
    }
    else if(read_pos == true && num_bytes != 15){
        read2 = true;
    }
}

void Hyperchromator::open(QString cal_path){
    float pos, FWHM;
    float wl;
    QString USB_Port;

    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        if((serialPortInfo.vendorIdentifier()==1659) && (serialPortInfo.productIdentifier()==8963)){
            USB_Port = serialPortInfo.portName();
        }
    }
    serial_port.setPortName(USB_Port);
    serial_port.setBaudRate(QSerialPort::Baud115200);
    serial_port.setDataBits(QSerialPort::Data8);
    serial_port.setParity(QSerialPort::NoParity);
    serial_port.setStopBits(QSerialPort::TwoStop);
    serial_port.setFlowControl(QSerialPort::NoFlowControl);
    bool error_sp = serial_port.open(QIODevice::ReadWrite);

    QFile file(cal_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Error Opening File.";
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
       QString line = in.readLine();
       QTextStream wl_cal_A(&line);
       wl_cal_A >> wl >> FWHM >> pos;
       wl_buffer[lines] = wl;
       pos_buffer[lines] = pos;
       lines++;
    }

    if(error_sp == false){ qDebug() << "Error trying to open port"; }
    qDebug() << "Port Opened";
    reset();
    qDebug() << "Homing...";
    timer.singleShot(20000, this, SLOT(initSpeed()));

}

void Hyperchromator::initSpeed(){
    qDebug() << "Set Initial Speed";
    setSpeed("250");
    timer.singleShot(1000, this, SLOT(initWL()));
}

void Hyperchromator::initWL(){
    qDebug() << "Set Initial Wavelength";
    setWL("500");
}

void Hyperchromator::close(){
    serial_port.close();
    qDebug() << "Port Closed";
}

void Hyperchromator::openShutter(){
    const unsigned char msg_open[8] = { 0x06, 0x0F, 0xF0, 0x74, 0x01, 0x40, 0xA1, 0x5B };
    int n = serial_port.write((char*)msg_open, 8);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Shutter Opened";
}

void Hyperchromator::closeShutter(){
    const unsigned char msg_close[8] = { 0x06, 0x0F, 0xF0, 0x74, 0x01, 0x40, 0xAA, 0x64 };
    int n = serial_port.write((char*)msg_close, 8);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Shutter Closed";
}

void Hyperchromator::setWL(QString wl_input_str){
    float pos_output, pos_floor, pos_ceil;
    float wl_floor, wl_ceil;

    wl_input = wl_input_str.toFloat();

    if(fmod(wl_input, 50) != 0.0){
        wl_floor = floor(wl_input/50)*50;
        wl_ceil = ceil(wl_input/50)*50;

        pos_floor = getPos(wl_floor);
        pos_ceil = getPos(wl_ceil);

        pos_output = pos_floor + (wl_input-wl_floor)*((pos_ceil-pos_floor)/(wl_ceil-wl_floor));
    }
    else{
        pos_output = getPos(wl_input);
    }
    setPos(pos_output);

}

float Hyperchromator::getPos(float wl){
    float pos;
    for(int i=0; i < lines; i++){
        if(wl_buffer[i] == wl){pos = pos_buffer[i];}
    }
    return pos;
}

void Hyperchromator::getWL(){
    qDebug() << "Current Wavelength: " << wl_input;
}

void Hyperchromator::setPos(float pos){
    unsigned char msg_pos[10] = { 0x08, 0x0F,0xF0, 0x24,0x9E };
    char    str_f[15];
    char    *token;
    long    int32;

    int ret = snprintf(str_f, sizeof str_f, "%f", pos);
    token = strtok(str_f, ".");

    // To store as 4 bytes: char * ->  int 32
    int32 = stol(token,nullptr,0);

    msg_pos[7] = int32 >> 24;
    msg_pos[8] = int32 >> 16;
    msg_pos[5] = int32 >> 8;
    msg_pos[6] = int32;

    for(int i=0; i < 9; i++){
        msg_pos[9] += msg_pos[i];
    }

    int n = serial_port.write((char*)msg_pos, 10);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Changing wavelength to " << wl_input;
    timer.singleShot(100, this, SLOT(updatePos()));
}

void Hyperchromator::updatePos(){
    const unsigned char msg_upd[6] = { 0x04, 0x0F, 0xF0, 0x01, 0x08, 0x0C };
    int n = serial_port.write((char*)msg_upd, 6);
    if(n == -1){ qDebug() << "Writting Fail"; }
    //qDebug() << "Position Updated";
}

void Hyperchromator::readPos(){
    read_pos = true;
    const unsigned char msg_read[10] = { 0x08, 0x0F, 0xF0, 0xB0, 0x05, 0x0F, 0xF1, 0x02, 0xB2, 0x70 };
    int n = serial_port.write((char*)msg_read, 10);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Reading Position of motor...";
}

void Hyperchromator::reset(){
    const unsigned char msg_reset[6] = { 0x04, 0x0F, 0xF0, 0x04, 0x02, 0x09 };
    int n = serial_port.write((char*)msg_reset, 6);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Reset";
    wl_input = 0;
}

void Hyperchromator::setSpeed(QString speed_str){
    float speed = speed_str.toFloat();
    unsigned char msg_speed[10] = { 0x08, 0x0F,0xF0, 0x24,0xA0 };
    char    str_f[15];
    char    *token;
    long    int32;

    int ret = snprintf(str_f, sizeof str_f, "%f", speed);
    token = strtok(str_f, ".");

    // To store as 4 bytes: char * ->  int 32
    int32 = stol(token,nullptr,0);

    msg_speed[7] = int32 >> 8;
    msg_speed[8] = int32;
    msg_speed[5] = 0x00;
    msg_speed[6] = 0x00;

    for(int i=0; i < 9; i++){
        msg_speed[9] += msg_speed[i];
    }

    int n = serial_port.write((char*)msg_speed, 10);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Changing speed to " << speed;
}

void Hyperchromator::filter1(){
    const unsigned char msg_f1[8] = { 0x06, 0x0F, 0xF0, 0x74, 0x01, 0x40, 0x7D, 0x37 };
    int n = serial_port.write((char*)msg_f1, 8);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Filter 1: no_filter";
}

void Hyperchromator::filter2(){
    const unsigned char msg_f2[8] = { 0x06, 0x0F, 0xF0, 0x74, 0x01, 0x40, 0x86, 0x40 };
    int n = serial_port.write((char*)msg_f2, 8);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Filter 2: LP_345nm";
}

void Hyperchromator::filter3(){
    const unsigned char msg_f3[8] = { 0x06, 0x0F, 0xF0, 0x74, 0x01, 0x40, 0x8F, 0x49 };
    int n = serial_port.write((char*)msg_f3, 8);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Filter 3: LP_630nm";
}

void Hyperchromator::filter4(){
    const unsigned char msg_f4[8] = { 0x06, 0x0F, 0xF0, 0x74, 0x01, 0x40, 0x98, 0x52 };
    int n = serial_port.write((char*)msg_f4, 8);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Filter 4: LP_1000nm";
}

void Hyperchromator::increment(){
    const unsigned char msg_inc[8] = { 0x06, 0x0F, 0xF0, 0x74, 0x01, 0x40, 0xFA, 0xB4 };
    int n = serial_port.write((char*)msg_inc, 8);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Offset incremented";
}

void Hyperchromator::decrement(){
    const unsigned char msg_dec[8] = { 0x06, 0x0F, 0xF0, 0x74, 0x01, 0x41, 0x03, 0xBE };
    int n = serial_port.write((char*)msg_dec, 8);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Offset decremented";
}

void Hyperchromator::clear(){
    const unsigned char msg_clear[8] = { 0x06, 0x0F, 0xF0, 0x74, 0x01, 0x41, 0x0C, 0xC7 };
    int n = serial_port.write((char*)msg_clear, 8);
    if(n == -1){ qDebug() << "Writting Fail"; }
    qDebug() << "Offsets cleared";
}
