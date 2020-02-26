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
    hdop->setText("HDOP : " + QString::number(cm.getData("HDOP")));
    latittude->setText("latittude : " + QString::number(cm.getData("Latittude")));
    longitude->setText("longitude : " + QString::number(cm.getData("Longitude")));
    lat_next_point->setText("lat_next_point : " + QString::number(cm.getData("Lat_next_point")));
    lon_next_point->setText("lon_next_point : " + QString::number(cm.getData("Lon_next_point")));
    lat_prev_point->setText("lat_prev_point : " + QString::number(cm.getData("Lat_prev_point")));
    lon_prev_point->setText("lon_prev_point : " + QString::number(cm.getData("Lon_prev_point")));
    wpt_angle->setText("wpt_angle : " + QString::number(cm.getData("Wpt_angle")));
    wpt_dist->setText("wpt_dist : " + QString::number(cm.getData("Wpt_dst")));
    Index_wpt->setText("Index_wpt : " + QString::number(cm.getData("Index_wpt")));

    speed->setText("speed : " + QString::number(cm.getData("Vitesse")));
    heading->setText("heading : " + QString::number(cm.getData("Cap")));

    reg_angle->setText("reg_angle : " + QString::number(cm.getData("Angle_regulateur")));
    winglet_pos->setText("winglet_pos : " + QString::number(cm.getData("Pos_aile")));
    battery->setText("battery : " + QString::number(cm.getData("Battery")));

    corridor_width->setText("corridor_width : " + QString::number(cm.getData("Corridor_width")));
    ecart_axe->setText("ecart_axe : " + QString::number(cm.getData("ecart_axe")));
    Presence_couloir->setText("Presence_couloir : " + QString::number(cm.getData("Presence_couloir")));


    int lat = cm.getData("Latittude");
    int lon = cm.getData("Longitude");
    int lat_next_p = cm.getData("Lat_next_point");
    int lon_next_p = cm.getData("Lon_next_point");
    int lat_prev_p = cm.getData("Lat_prev_point");
    int lon_prev_p = cm.getData("Lon_prev_point");

    if (lat != 404 && lon != 404 && lat != 0 && lon != 0){
    ligne1->setLine((lon-lon_ofset)/scale-5, -(lat-lat_ofset)/scale, (lon-lon_ofset)/scale+5, -(lat-lat_ofset)/scale);
    ligne2->setLine((lon-lon_ofset)/scale, -(lat-lat_ofset)/scale-5, (lon-lon_ofset)/scale, -(lat-lat_ofset)/scale+5);
    }

    if (lat_next_p != 404 && lon_next_p != 404 && lat_next_p != 0 && lon_next_p != 0){
       wpt_circle1->setPos(-2.5+(lon_next_p-lon_ofset)/scale, -2.5-(lat_next_p-lat_ofset)/scale);
       label_wpt1->setPos((lon_next_p-lon_ofset)/scale, -20-(lat_next_p-lat_ofset)/scale);
        if (lat_prev_p != 404 && lon_prev_p != 404 && lat_prev_p != 0 && lon_prev_p != 0){
           wpt_circle2->setPos(-2.5+(lon_prev_p-lon_ofset)/scale, -2.5-(lat_prev_p-lat_ofset)/scale);
           label_wpt2->setPos((lon_prev_p-lon_ofset)/scale, -20-(lat_prev_p-lat_ofset)/scale);
           ligne3->setLine((lon_next_p-lon_ofset)/scale, -(lat_next_p-lat_ofset)/scale, (lon_prev_p-lon_ofset)/scale, -(lat_prev_p-lat_ofset)/scale);
        }
    }
}

void MainWindow::setVarDisplay(QGridLayout * layout)
{
    QGridLayout * grid = new QGridLayout();

    hdop = new QLabel();
    grid->addWidget(hdop,0,0);
    latittude = new QLabel();
    grid->addWidget(latittude,1,0);
    longitude = new QLabel();
    grid->addWidget(longitude,2,0);
    lat_next_point = new QLabel();
    grid->addWidget(lat_next_point,3,0);
    lon_next_point= new QLabel();
    grid->addWidget(lon_next_point,4,0);
    lat_prev_point= new QLabel();
    grid->addWidget(lat_prev_point,5,0);
    lon_prev_point= new QLabel();
    grid->addWidget(lon_prev_point,6,0);
    wpt_angle = new QLabel();
    grid->addWidget(wpt_angle,7,0);
    wpt_dist = new QLabel();
    grid->addWidget(wpt_dist,8,0);
    Index_wpt = new QLabel();
    grid->addWidget(Index_wpt,9,0);
    speed = new QLabel();
    grid->addWidget(speed,10,0);
    heading = new QLabel();
    grid->addWidget(heading,11,0);
    reg_angle = new QLabel();
    grid->addWidget(reg_angle,12,0);
    winglet_pos = new QLabel();
    grid->addWidget(winglet_pos,13,0);
    battery = new QLabel();
    grid->addWidget(battery,14,0);
    corridor_width = new QLabel();
    grid->addWidget(corridor_width,15,0);
    ecart_axe = new QLabel();
    grid->addWidget(ecart_axe,16,0);
    Presence_couloir = new QLabel();
    grid->addWidget(Presence_couloir,17,0);

    layout->addLayout(grid,0,0);
}

MainWindow::MainWindow()
{
    QWidget *zoneCentrale = new QWidget;

    cm.openSerialPort("//./COM6");

    // lecture de fichier
    QPolygon fond_carte = createBackground();
    scene.addPolygon(fond_carte);

    // draw the cross representing the boat
    ligne1 = scene.addLine(QLine(-5,0,5,0));
    ligne2 = scene.addLine(QLine(0,-5,0,5));

    // draw the circle for the first wpt
    label_wpt1 = scene.addText("next wpt");
    wpt_circle1 = scene.addEllipse(0,0,10,10);
    label_wpt2 = scene.addText("prev wpt");
    wpt_circle2 = scene.addEllipse(0,0,10,10);
    ligne3 = scene.addLine(QLine(0,1,0,2));

    view = new QGraphicsView(&scene);
    view->show();

    QGridLayout  *layout = new QGridLayout();

    setVarDisplay(layout);

    layout->addWidget(view,1,2);
    zoneCentrale->setLayout(layout);
    setCentralWidget(zoneCentrale);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::updateView));
    timer->start(1000);
}
