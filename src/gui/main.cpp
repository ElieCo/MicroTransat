
#include <QApplication>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDebug>
#include "mainwindow.h"
#include "comm_manager.h"
#include <QFile>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // display window
    QGraphicsScene scene;
    scene.addText("Hello world !");

    // Display map background
    QPolygon fond_carte;
    fond_carte << QPoint(100, 100);
    fond_carte << QPoint(100, 0);
    fond_carte << QPoint(0, 100);
    scene.addPolygon(fond_carte);

    // lecture de fichier
    QString text;
    QFile fichier("./MicroTransat/src/gui/resources/carte_lac.csv");

    if(fichier.open(QIODevice::ReadOnly | QIODevice::Text))
    {
         text = fichier.readAll();
         fichier.close();
    }
    else text = "Impossible d'ouvrir le fichier !";
    qDebug() << text;


    QGraphicsView vue(&scene);
    vue.show();

    //return app.exec();

//    CommManager cm;
//    cm.openSerialPort("//./COM6");
//    cm.closeSerialPort();
//
    return a.exec();
}
