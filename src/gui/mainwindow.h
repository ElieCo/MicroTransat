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
    CommManager cm;
    QGraphicsTextItem * label;
};

#endif
