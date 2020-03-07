#include "comm_manager.h"
#include "QTimer"

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
          m_serial->setBaudRate(QSerialPort::Baud115200);
          m_serial->setStopBits(QSerialPort::OneStop);
          m_serial->setParity(QSerialPort::NoParity);
          m_serial->setDataBits(QSerialPort::Data8);
          m_serial->setFlowControl(QSerialPort::NoFlowControl);

          // definition de la liste des entrée (oui c'est un peu caca)
          header = QStringList({"Battery", "Time", "HDOP", "Vitesse", "Cap", "Angle_regulateur", "Asserv_regulateur", "Pos_aile", "Cap_moy", "Latittude", "Longitude", "Lat_next_point", "Lon_next_point", "Lat_prev_point", "Lon_prev_point", "Corridor_width", "Wpt_angle", "Wpt_dst", "ecart_axe", "Presence_couloir", "Index_wpt"});
          for (int i=0; i<header.size(); i++){
              m_serialData.insert(header.at(i), 0);
          }
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
    if (dataList.length() == header.length()){
        for (int i = 0; i < header.length(); i++){
            m_serialData[header.at(i)] = dataList[i].toInt();
        }
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
    if (data.contains('~')){   // fin du message detectee
        m_cache += data;
        m_cache.remove(QChar('\r'), Qt::CaseInsensitive);
        m_cache.remove(QChar('\n'), Qt::CaseInsensitive);
        m_cache.remove(QChar('~'), Qt::CaseInsensitive);
        decryptMsg(m_cache);
        qDebug() << m_cache;
        m_cache = "";
    }
    else {
        m_cache += data;
    }
}

void CommManager::send(QString text)
{
    m_serial->flush();
    m_serial->write(text.toStdString().c_str());
}