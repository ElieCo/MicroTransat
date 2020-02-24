#include "MainWindow.h"
#include <QFile>
#include <QGraphicsTextItem>

/**
 *  createBackground -> Read the map background file and convert it into a polygon
**/
QPolygon MainWindow::createBackground(){
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

         lat_ofset = (lat_max - lat_min)/2 + lat_min;
         lon_ofset = (lon_max - lon_min)/2 + lon_min;
         scale = 0;
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

void MainWindow::updateView()
{
    label_hdop->setPlainText("HDOP : " + QString::number(cm.getData("HDOP")));
    label_speed->setPlainText("Speed : " + QString::number(cm.getData("Vitesse")));
    label_reg_angle->setPlainText("Reg angle : " + QString::number(cm.getData("Angle_regulateur")));
    label_battery->setPlainText("Battery : " + QString::number(cm.getData("Battery")));
    int lat = cm.getData("Latittude");
    int lon = cm.getData("Longitude");

    if (lat != 404 && lon != 404){
    ligne1->setLine((lon-lon_ofset)/scale-5, -(lat-lat_ofset)/scale, (lon-lon_ofset)/scale+5, -(lat-lat_ofset)/scale);
    ligne2->setLine((lon-lon_ofset)/scale, -(lat-lat_ofset)/scale-5, (lon-lon_ofset)/scale, -(lat-lat_ofset)/scale+5);
    }
}

MainWindow::MainWindow()
{
    QWidget *zoneCentrale = new QWidget;

    cm.openSerialPort("//./COM6");

    // lecture de fichier
    QPolygon fond_carte = createBackground();
    scene.addPolygon(fond_carte);
    scene.addText("Lac du ter");

    ligne1 = scene.addLine(QLine(-5,0,5,0));
    ligne2 = scene.addLine(QLine(0,-5,0,5));

    label_hdop = new QGraphicsTextItem();
    label_speed = new QGraphicsTextItem();
    label_reg_angle = new QGraphicsTextItem();
    label_battery = new QGraphicsTextItem();

    label_hdop->setPos(-200, 100);
    label_speed->setPos(-100, 100);
    label_reg_angle->setPos(0, 100);
    label_battery->setPos(100, 100);

    scene.addItem(label_hdop);
    scene.addItem(label_speed);
    scene.addItem(label_reg_angle);
    scene.addItem(label_battery);

    view = new QGraphicsView(&scene);
    view->show();

    QFormLayout *layout = new QFormLayout;
    layout->addWidget(view);
    zoneCentrale->setLayout(layout);
    setCentralWidget(zoneCentrale);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::updateView));
    timer->start(1000);
}
