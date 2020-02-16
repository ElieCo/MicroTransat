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
    if(!msg.contains(";"))
        return;

    QStringList dataList = msg.split(";");
    bool isNotFirstLine = false;
    dataList[0].toInt(&isNotFirstLine);
    if (isNotFirstLine && m_firstLine.length() >= dataList.length()) {
        for (int i = 0; i < dataList.length(); i++){
            m_serialData[m_firstLine[i]] = dataList[i].toInt();
        }
    } else if(!isNotFirstLine && m_firstLine.length() == 0){
        for (int i = 0; i < dataList.length(); i++){
            if (!dataList[i].contains("\n")){
                m_serialData.insert(dataList[i], 0);
                m_firstLine.append(dataList[i]);
            }
        }
    }
    qDebug() << m_serialData;
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
