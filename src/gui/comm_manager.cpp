#include "comm_manager.h"

CommManager::CommManager(QObject *parent)
    : QObject(parent)
{
}

CommManager::~CommManager()
{
    if (m_serial) delete m_serial;
}

void CommManager::openSerialPort(QString nameport)
{
    m_serial = new QSerialPort();
    connect(m_serial,SIGNAL(readyRead()),this,SLOT(readData()));
    m_serial->setPortName(nameport);
    m_serial->open(QIODevice::ReadWrite);

     if( m_serial->isOpen()==false)
     {
          m_serial->clearError();
          qDebug() << "Port error", "Port: "+nameport;
          qDebug() << "Port error", "Vérifier nom du port \n Fermer tout programme utilisant la lisaison RS232 "+nameport;
      }
   else
     {
         qDebug() << "Port open "+nameport;
          m_serial->setBaudRate(QSerialPort::Baud9600);
          m_serial->setStopBits(QSerialPort::OneStop);
          m_serial->setParity(QSerialPort::NoParity);
          m_serial->setDataBits(QSerialPort::Data8);
          m_serial->setFlowControl(QSerialPort::NoFlowControl);
     }
}

void CommManager::closeSerialPort()
{
    m_serial->close();
}

void CommManager::decryptMsg(QString msg)
{
    QStringList dataList = msg.split(0x1F);
    if (dataList.length() == 2){
        m_serialData[dataList[0]] = dataList[1].toInt();
    }
}

int CommManager::getData(QString name)
{
    if (m_serialData.contains(name)){
        return  m_serialData[name];
    }
    else {
        return 404;
    }
}

void CommManager::readData()
{
    QByteArray data = m_serial->readAll();
    if (data.contains("\n")) {
        if (data.contains("\r")){
            m_cache += data.split('\r')[0];
        }
        decryptMsg(m_cache);
        m_cache = data.split('\n').last();
    }
    else if (!data.contains('\r')){
        m_cache += data;
    }
    else {
        m_cache += data.split('\r')[0];
    }
}

void CommManager::send(QString text)
{
    m_serial->write(text.toStdString().c_str());
}
