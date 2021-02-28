#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QDialog>
#include <QSerialPort>

namespace Ui {
class FileManager;
}

class FileManager : public QDialog
{
    Q_OBJECT

    enum LastCmd{
        NONE = 0,
        PING = 1,
        LS = 2,
        CAT = 3
    };

public:
    explicit FileManager(QWidget *parent = nullptr);
    ~FileManager();

    void openSerialPort(QString nameport);
    void closeSerialPort();

private:
    Ui::FileManager *ui;
    QSerialPort m_serial;
    bool m_connected;
    LastCmd m_last_cmd;
    QString m_cat_filename;
    QByteArray m_cat_buffer;

    void ping_react(QByteArray data);
    void ls_react(QString data);
    void cat_react(QByteArray data);
    void create_react(QByteArray data);

    void updateEnable();

public slots:
    void readData();

private slots:
    void ping();
    void ls();
    void cat();
    void rm();
    void create();

};

#endif // FILEMANAGER_H
