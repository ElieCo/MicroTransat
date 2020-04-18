#ifndef COMM_MANAGER_H
#define COMM_MANAGER_H

#include <QObject>
#include <QDebug>
#include <QMap>
#include <QSerialPort>

class CommManager : public QObject
{
    Q_OBJECT

public:
    explicit CommManager(QObject *parent = nullptr);
    ~CommManager();

    void openSerialPort(QString nameport);
    QStringList getHeader();
    QList<float> getFullList();
    void send();
    void setrequest(QString text);
    int getData(QString name);
    void setData(QString name, int value);
    void closeSerialPort();

private:
    void decryptMsg(QString msg);

    QMap<QString, float> m_serialData;
    QSerialPort *m_serial;
    QString m_cache;
    QStringList header;
    QString request;

public slots:
    void readData();

};

#endif // COMM_MANAGER_H
