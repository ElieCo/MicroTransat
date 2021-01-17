#include "comm_manager.h"
#include "QTimer"

CommManager::CommManager(QObject *parent)
    : QObject(parent)
{
    // definition de la liste des entrée (oui c'est un peu caca mais je vais revenir dessus !)
    header = QStringList({"Wpt_index","Behaviour","Time","Wpt_dist","Wpt_angle","Dist_to_axis","Cmd_helm","Regulator_angle","Wing_angle","Speed","Course","Average_course","Latitude","Longitude","HDOP","Battery","In_corridor","Fix","Gps_ready","Radio_controlled","Prev_element","Next_element"});
    for (int i=0; i<header.size(); i++){
        m_serialData.insert(header.at(i), 0);
    }
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
          //qDebug() << "Port error", "Port: "+nameport;
          //qDebug() << "Port error", "Vérifier nom du port \n Fermer tout programme utilisant la lisaison RS232 "+nameport;
      }
   else
     {
         qDebug() << "Port open "+nameport;
          m_serial->setBaudRate(QSerialPort::Baud115200);
          m_serial->setStopBits(QSerialPort::OneStop);
          m_serial->setParity(QSerialPort::NoParity);
          m_serial->setDataBits(QSerialPort::Data8);
          m_serial->setFlowControl(QSerialPort::NoFlowControl);
     }
}

QStringList CommManager::getHeader()
{
    return header;
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
            if (dataList[i].contains("AWA")){           // verrue pas belle mais on avait dit "only float by lora" !!
                dataList[i].remove("AWA-");
                QStringList a = dataList[i].split("/");
                dataList[i] = a[0].split(".")[0]+a[1].split(".")[0];
            }
            m_serialData[header.at(i)] = dataList[i].toFloat();
        }
        m_serialData["Latitude"] *=       1000000;
        m_serialData["Longitude"] *=      1000000;
        m_serialData["Lat_next_point"] *= 1000000;
        m_serialData["Lon_next_point"] *= 1000000;
        m_serialData["Lat_prev_point"] *= 1000000;
        m_serialData["Lon_prev_point"] *= 1000000;
    }
}

QList<float> CommManager::getFullList()
{
    QList<float> a;
    for (int i=0; i<header.count(); i++)
    {
        a.append(m_serialData[header.at(i)]);
    }
    return a;
}

float CommManager::getData(QString name)
{
    if (m_serialData.contains(name)){
        return  m_serialData[name];
    }
    else {
        return 404;
    }
}

void CommManager::setData(QString name, int value)
{
    m_serialData[name] = value;
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
        //qDebug() << "msg recu et assemble : " << m_cache;
        m_cache = "";
    }
    else {
        m_cache += data;
    }
}
void CommManager::setrequest(QString text)
{
    request = text;
}

void CommManager::send()
{
    if (request.length() == 0){
        request = "log";
    }
    m_serial->flush();
    m_serial->write(request.toStdString().c_str());

    request = "";
}

QStringList CommManager::openFile(QString file_name)
{
    file.setFileName(file_name);
    if (file.open(QFile::ReadOnly)) {
        // read header
        char buf[1024];
        qint64 lineLength = file.readLine(buf, sizeof(buf));
        if (lineLength != -1) {
            QString line = QString(buf);
            line.remove(QChar('\r'), Qt::CaseInsensitive);
            line.remove(QChar('\n'), Qt::CaseInsensitive);
            header = line.split(";");
            m_serialData.clear();
            for (int i=0; i<header.size(); i++){
                m_serialData.insert(header.at(i), 0);
            }
            return header;
        }
    }
    return QStringList();
}

void CommManager::readLine()
{
    char buf[1024];
    qint64 lineLength = file.readLine(buf, sizeof(buf));
    if (lineLength != -1) {
        QString line = QString(buf);
        line.remove(QChar('\r'), Qt::CaseInsensitive);
        line.remove(QChar('\n'), Qt::CaseInsensitive);
        decryptMsg(line);
    }
    else {
        file.close();
    }
}
