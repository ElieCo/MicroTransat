#include "mainwindow.h"
#include "ui_mainwindow.h"
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
             scale = (lat_max - lat_min)/500;
         }
         else {
             scale = (lon_max - lon_min)/200;
         }

         for (int i = 0; i < lat.length(); i++){
             fond_carte << QPoint((lon[i]-lon_ofset)/scale, -(lat[i]-lat_ofset)/scale);
         }
    }

    else raw_background = "Impossible d'ouvrir le fichier !";

    return fond_carte;
}

void MainWindow::updateRawData()
{
    QList<float> newList = cm.getFullList();
    for (int i=0; i< raw_values.count(); i++){
        raw_values[i]->setText(header[i]+" : " + newList.at(i));
    }
}

void MainWindow::updateBoatPosition()
{
    int lat = cm.getData("Latitude");
    int lon = cm.getData("Longitude");


    if ((cm.getData("Lat_prev_point") == 404 || cm.getData("Lat_prev_point") == 0) && lat != 404 && lat !=0) {
        cm.setData("Lat_prev_point", lat);
        lat_prev_p = lat;
    }
    if ((cm.getData("Lon_prev_point") == 404 || cm.getData("Lon_prev_point") == 0) && lon != 404 && lon !=0) {
        cm.setData("Lon_prev_point", lon);
        lon_prev_p = lon;
    }

    if (lat_next_p != cm.getData("Lat_next_point") && lat_next_p != 0 && lat_next_p != 404){
        lat_prev_p = lat_next_p;
        cm.setData("Lat_prev_point", lat_prev_p);
    }
    lat_next_p = cm.getData("Lat_next_point");

    if (lon_next_p != cm.getData("Lon_next_point") && lon_next_p != 0 && lon_next_p != 404){
        lon_prev_p = lon_next_p;
        cm.setData("Lat_prev_point", lon_prev_p);
    }
    lon_next_p = cm.getData("Lon_next_point");

    if (lat != 404 && lon != 404 && lat != 0 && lon != 0){
        if (track.length()>0){
            track.lineTo((lon-lon_ofset)/scale,-(lat-lat_ofset)/scale);
        }
        else {
            track.moveTo((lon-lon_ofset)/scale,1-(lat-lat_ofset)/scale);
            track.lineTo((lon-lon_ofset)/scale,-(lat-lat_ofset)/scale);
        }
        ligne1->setLine((lon-lon_ofset)/scale-5, -(lat-lat_ofset)/scale, (lon-lon_ofset)/scale+5, -(lat-lat_ofset)/scale);
        ligne2->setLine((lon-lon_ofset)/scale, -(lat-lat_ofset)/scale-5, (lon-lon_ofset)/scale, -(lat-lat_ofset)/scale+5);
    }

    if (lat_next_p != 404 && lon_next_p != 404 && lat_next_p != 0 && lon_next_p != 0){
        wpt_circle1->setPos(-2.5+(lon_next_p-lon_ofset)/scale, -5-(lat_next_p-lat_ofset)/scale);
        label_wpt1->setPos((lon_next_p-lon_ofset)/scale, -20-(lat_next_p-lat_ofset)/scale);

        if (lat_prev_p != 404 && lon_prev_p != 404 && lat_prev_p != 0 && lon_prev_p != 0){
           wpt_circle2->setPos(-2.5+(lon_prev_p-lon_ofset)/scale, -5-(lat_prev_p-lat_ofset)/scale);
           label_wpt2->setPos((lon_prev_p-lon_ofset)/scale, -20-(lat_prev_p-lat_ofset)/scale);
           ligne3->setLine((lon_next_p-lon_ofset)/scale, -(lat_next_p-lat_ofset)/scale, (lon_prev_p-lon_ofset)/scale, -(lat_prev_p-lat_ofset)/scale);
        }

        int heading_ = cm.getData("Course");
        if (heading_ != 404)
        {
            label_cap->setPos(sin(M_PI*heading_/180)*40+(lon-lon_ofset)/scale, -cos(M_PI*heading_/180)*40-(lat-lat_ofset)/scale);
            cap->setLine((lon-lon_ofset)/scale, -(lat-lat_ofset)/scale, sin(M_PI*heading_/180)*40+(lon-lon_ofset)/scale, -cos(M_PI*heading_/180)*40-(lat-lat_ofset)/scale);
        }
        int wind_angle = cm.getData("Regulator_angle");
        if (wind_angle != 404)
        {
            if (wind_angle < 0){
                wind_angle += 180;
            }
            wind_angle = heading_ - wind_angle;

            if (wind_angle > 360)
            {
                wind_angle -= 360;
            }

            label_wind->setPos(sin(M_PI*wind_angle/180)*40+(lon-lon_ofset)/scale, -cos(M_PI*wind_angle/180)*40-(lat-lat_ofset)/scale);
            wind->setLine((lon-lon_ofset)/scale, -(lat-lat_ofset)/scale, sin(M_PI*wind_angle/180)*40+(lon-lon_ofset)/scale, -cos(M_PI*wind_angle/180)*40-(lat-lat_ofset)/scale);
        }
    }
}

void MainWindow::updateView()
{
    cm.send();   // send a command to the boat (the command sent is defined by the last call of setrequest()).

    updateRawData();
    //updateBoatPosition();
    path->setPath(track);
}

void MainWindow::setVarDisplay()
{
    for (int i=0; i<header.size(); i++){
        QLabel *raw = new QLabel(header.at(i)+" : ");
        ui->rawDataLayout->addWidget(raw);
        raw_values << raw;
    }
}

void MainWindow::setButtonDisplay(QGridLayout * layout)
{
    QGridLayout * grid = new QGridLayout();

    val_selection = new QSpinBox();
    val_selection->setMaximum(99999999);
    val_selection->setMinimum(0);
    connect(val_selection, SIGNAL (valueChanged(int)), this, SLOT (update_val(int)));
    grid->addWidget(val_selection,0,0);

    test_button = new QPushButton("My Button", this);
    test_button->setGeometry(QRect(QPoint(100, 100),QSize(200, 50)));
    connect(test_button, SIGNAL (released()), this, SLOT (handleButton()));
    grid->addWidget(test_button,0,1);


    // track display
    resetTrack = new QPushButton("Reset Track", this);
    connect(resetTrack, SIGNAL (released()), this, SLOT (resetHit()));
    grid->addWidget(resetTrack,1,0);

    layout->addLayout(grid, 0, 1);
}

void MainWindow::update_val(int i){
        qDebug()<< "valeur de la box : "<< i;
}

void MainWindow::resetHit()
{
   track.clear();
}

void MainWindow::handleButton()
{
   test_button->setText("Example");
   //qDebug()<< "valeur de la box : "<< val_selection->value();
   cm.setrequest("c"+QString::number(val_selection->value()));
}



MainWindow::MainWindow() :
    lat_next_p(0)
    , lon_next_p(0)
    , lat_prev_p(0)
    , lon_prev_p(0)
    ,ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    cm.openSerialPort("//./COM6");

    header = cm.getHeader();

    // lecture de fichier
    scene.addPolygon(createBackground());

    // add the boat track in the scetch
    path = scene.addPath(track);

    // draw the cross representing the boat
    ligne1 = scene.addLine(QLine(-5,0,5,0));
    ligne2 = scene.addLine(QLine(0,-5,0,5));

    label_cap = scene.addText("heading");
    cap = scene.addLine(QLine(0,0,0,1));
    label_wind = scene.addText("supposed wind direction");
    wind = scene.addLine(QLine(0,0,0,1));

    // draw the circle for the first wpt
    label_wpt1 = scene.addText("next wpt");
    wpt_circle1 = scene.addEllipse(0,0,10,10);
    label_wpt2 = scene.addText("prev wpt");
    wpt_circle2 = scene.addEllipse(0,0,10,10);
    ligne3 = scene.addLine(QLine(0,1,0,2));

    ui->graphicMap->setScene(&scene);

    setVarDisplay();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateView);
    timer->start(1500);
}
