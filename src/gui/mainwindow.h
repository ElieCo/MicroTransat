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
    void setVarDisplay(QGridLayout  *);


    QGraphicsScene scene;
    QGraphicsView * view;
    QGraphicsLineItem * ligne1;
    QGraphicsLineItem * ligne2;
    QGraphicsLineItem * ligne3;
    QGraphicsLineItem * cap;
    QGraphicsLineItem * wind;

    QGraphicsEllipseItem * wpt_circle1;
    QGraphicsEllipseItem * wpt_circle2;
    QGraphicsTextItem * label_wpt1;
    QGraphicsTextItem * label_wpt2;
    QGraphicsTextItem * label_cap;
    QGraphicsTextItem * label_wind;

    CommManager cm;

    QLabel * hdop;
    QLabel * latittude;
    QLabel * longitude;
    QLabel * lat_next_point;
    QLabel * lon_next_point;
    QLabel * lat_prev_point;
    QLabel * lon_prev_point;
    QLabel * wpt_angle;
    QLabel * wpt_dist;
    QLabel * Index_wpt;

    QLabel * speed;
    QLabel * heading;

    QLabel * reg_angle;
    QLabel * winglet_pos;
    QLabel * battery;

    QLabel * corridor_width;
    QLabel * ecart_axe;
    QLabel * Presence_couloir;

    int lat_ofset;
    int lon_ofset;
    int scale;
};

#endif
