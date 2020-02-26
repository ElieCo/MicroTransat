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
    QGraphicsEllipseItem * wpt_circle;
    QGraphicsTextItem * label_wpt;

    CommManager cm;

    QLabel * hdop;
    QLabel * latittude;
    QLabel * longitude;
    QLabel * lat_next_point;
    QLabel * lon_next_point;
    QLabel * wpt_angle;
    QLabel * wpt_dist;
    QLabel * Index_wpt;

    QLabel * speed;
    QLabel * heading;

    QLabel * reg_angle;
    QLabel * winglet_pos;
    QLabel * battery;

    QLabel * ecart_axe;
    QLabel * Presence_couloir;

    int lat_ofset;
    int lon_ofset;
    int scale;
};

#endif
