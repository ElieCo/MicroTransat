
#include <QApplication>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDebug>
#include "mainwindow.h"
#include "comm_manager.h"
#include <QFile>

/**
 *  createBackground -> Read the map background file and convert it into a polygon
**/
QPolygon createBackground(){
    QPolygon fond_carte;
    QString raw_background;
    QFile fichier("../src/gui/resources/carte_lac.csv");

    if(fichier.open(QIODevice::ReadOnly | QIODevice::Text))
    {
         raw_background = fichier.readAll();
         fichier.close();

         // convertion from string to list of lat and lon
         QStringList list = raw_background.split("\n");
         QList<int> lat;
         QList<int> lon;
         int lat_min = 0;
         int lat_max = 0;
         int lon_min = 0;
         int lon_max = 0;
         for (int i = 0; i < list.length(); i++){
            lat.append(list[i].split(";")[0].toInt());
            lon.append(list[i].split(";")[1].toInt());

            if (i ==0){
                lat_min = lat.last();
                lat_max = lat.last();
                lon_min = lon.last();
                lon_max = lon.last();
            }
            if (lat.last()>lat_max){
                lat_max = lat.last();
            }
            if (lat.last()<lat_min){
                lat_min = lat.last();
            }
            if (lon.last()>lon_max){
                lon_max = lon.last();
            }
            if (lon.last()<lon_min){
                lon_min = lon.last();
            }
         }

         int lat_ofset = (lat_max - lat_min)/2 + lat_min;
         int lon_ofset = (lon_max - lon_min)/2 + lon_min;
         int scale = 0;
         if (lat_min - lat_max > lon_min - lon_max){
             scale = (lat_max - lat_min)/200;
         }
         else {
             scale = (lon_max - lon_min)/200;
         }

         for (int i = 0; i < lat.length(); i++){
             //qDebug() << lon[i] << "   " << scale << "   " << lon_ofset;
             fond_carte << QPoint((lon[i]-lon_ofset)/scale, -(lat[i]-lat_ofset)/scale);
         }
    }

    else raw_background = "Impossible d'ouvrir le fichier !";

    return fond_carte;
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // display window
    QGraphicsScene scene;
    scene.addText("Hello world !");

    // lecture de fichier
    QPolygon fond_carte = createBackground();

    scene.addPolygon(fond_carte);

    QGraphicsView vue(&scene);
    vue.show();

    //return app.exec();

//    CommManager cm;
//    cm.openSerialPort("//./COM6");
//    cm.closeSerialPort();
//
    return a.exec();
}
