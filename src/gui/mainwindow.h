#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>

#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSlider>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:

private:
  Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
