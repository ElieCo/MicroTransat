#ifndef HEADER_MAINWINDOW
#define HEADER_MAINWINDOW

#include <QtWidgets>
#include "comm_manager.h"

class MainWindow : public QMainWindow
{
    public:
    MainWindow();

    private:
    QPolygon createBackground();
    void updateView();
    QGraphicsScene scene;
    QGraphicsView * view;
    QGraphicsLineItem * ligne1;
    QGraphicsLineItem * ligne2;
    CommManager cm;
    QGraphicsTextItem * label_hdop;
    QGraphicsTextItem * label_speed;
    QGraphicsTextItem * label_reg_angle;
    QGraphicsTextItem * label_battery;
    int lat_ofset;
    int lon_ofset;
    int scale;
};

#endif
