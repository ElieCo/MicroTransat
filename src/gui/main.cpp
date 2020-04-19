
#include <QApplication>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow window;
    window.show();

    //    cm.closeSerialPort();
    return a.exec();
}
