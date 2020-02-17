#include <QGuiApplication>
#include <QDebug>

#include "mainwindow.h"

#include "comm_manager.h"

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    CommManager cm;
    cm.openSerialPort("/dev/ttyUSB0");
    cm.closeSerialPort();
//
//    return a.exec();
}
