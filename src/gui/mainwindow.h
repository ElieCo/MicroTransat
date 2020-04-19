#ifndef HEADER_MAINWINDOW
#define HEADER_MAINWINDOW

#include <QtWidgets>
#include "comm_manager.h"

namespace Ui {
    class MainWindow;
}


class MainWindow : public QMainWindow
{
        Q_OBJECT
    public:
    MainWindow();

    private slots:
    void handleButton();

    private:
    Ui::MainWindow *ui;
    QPolygon createBackground();
    void updateRawData();
    void updateBoatPosition();
    void setVarDisplay();
    void setButtonDisplay(QGridLayout *);
    void clearLayout(QLayout *);

    QGraphicsScene scene;
    QGraphicsView * view;
    QGraphicsPathItem * path;
    QPainterPath track;
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

    QPushButton * resetTrack;
    QPushButton * test_button;
    QSpinBox * val_selection;

    bool replay_mode;

    QList<QLabel*> raw_values;

    CommManager cm;

    QStringList header;

    int lat_ofset;
    int lon_ofset;
    int scale;

    int lat_next_p, lon_next_p;
    int lat_prev_p, lon_prev_p;

public slots:

    void updateView();
    void openDialBox();
};

#endif
