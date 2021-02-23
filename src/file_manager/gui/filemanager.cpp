#include "filemanager.h"
#include "ui_filemanager.h"
#include <QDebug>
#include <QtWidgets>

FileManager::FileManager(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FileManager)
    , m_connected(false)
    , m_last_cmd(NONE)
    , m_cat_filename("")
    , m_cat_buffer("")
{
    ui->setupUi(this);

    ui->port_lineEdit->setText("/dev/ttyACM0");

    connect(&m_serial, &QSerialPort::readyRead, this, &FileManager::readData);
    connect(ui->ping_pushButton, &QPushButton::clicked, this, &FileManager::ping);
    connect(ui->ls_pushButton, &QPushButton::clicked, this, &FileManager::ls);
    connect(ui->cat_pushButton, &QPushButton::clicked, this, &FileManager::cat);
    connect(ui->rm_pushButton, &QPushButton::clicked, this, &FileManager::rm);
    connect(ui->create_pushButton, &QPushButton::clicked, this, &FileManager::create);
}

FileManager::~FileManager()
{
    delete ui;
}

void FileManager::openSerialPort(QString nameport)
{
    m_serial.setPortName(nameport);
    m_serial.open(QIODevice::ReadWrite);

    if(!m_serial.isOpen()) {
        m_serial.clearError();
        qDebug() << "Fail to open port" << nameport;

    } else {
        qDebug() << "Port open" << nameport;
        m_serial.setBaudRate(QSerialPort::Baud115200);
        m_serial.setStopBits(QSerialPort::OneStop);
        m_serial.setParity(QSerialPort::NoParity);
        m_serial.setDataBits(QSerialPort::Data8);
        m_serial.setFlowControl(QSerialPort::NoFlowControl);
    }
}

void FileManager::closeSerialPort()
{
    m_serial.close();
}

void FileManager::ping()
{
    openSerialPort(ui->port_lineEdit->text());

    if (m_serial.isOpen()){
        m_serial.write("ping\r");
        m_last_cmd = PING;
    }
}

void FileManager::ls()
{
    if (m_serial.isOpen()){
        m_serial.write("ls\r");
        m_last_cmd = LS;
    }
}

void FileManager::cat()
{

    if (m_serial.isOpen()){
        QString filename = QFileDialog::getSaveFileName(this);
        if (!filename.isEmpty()){
            QList<QListWidgetItem*> items = ui->files_listWidget->selectedItems();
            if (items.length() > 0){
                QFileInfo fileInfo(filename);
                m_cat_filename = fileInfo.dir().path() + "/" + items[0]->text();

                m_serial.write(QString("cat %1\r").arg(items[0]->text()).toUtf8());
                m_last_cmd = CAT;
            }
        }
    }
}

void FileManager::rm()
{
    if (m_serial.isOpen()){
        QList<QListWidgetItem*> items = ui->files_listWidget->selectedItems();
        if (items.length() > 0){
            m_serial.write(QString("rm %1\r").arg(items[0]->text()).toUtf8());
            ls();
        }
    }
}

void FileManager::create()
{

}

void FileManager::ping_react(QByteArray data)
{
    m_connected = data.contains("pong");
    updateEnable();
}

void FileManager::ls_react(QString data)
{
    data.remove("\r");
    QList<QString> list = data.split("\n");
    ui->files_listWidget->clear();
    foreach (QString str, list)
        if (str.length() > 0)
            ui->files_listWidget->addItem(str);

}

void FileManager::cat_react(QByteArray data)
{
    m_cat_buffer += data;
    if (m_cat_buffer.contains("<<END OF FILE>>") && !m_cat_filename.isEmpty()){
        QFile file(m_cat_filename);
        file.open(QFile::WriteOnly);
        QString d = m_cat_buffer;
        d.remove("<<END OF FILE>>");
        file.write(d.toUtf8());
        file.flush();
        file.close();
        m_cat_buffer = "";
    }
}

void FileManager::create_react(QByteArray data)
{

}

void FileManager::updateEnable()
{
    ui->ls_pushButton->setEnabled(m_connected);
    ui->cat_pushButton->setEnabled(m_connected);
    ui->rm_pushButton->setEnabled(m_connected);
    ui->create_pushButton->setEnabled(m_connected);
    ui->files_listWidget->setEnabled(m_connected);
}

void FileManager::readData()
{
    QByteArray data = m_serial.readAll();
//    qDebug() << "recieve" << data;

    switch (m_last_cmd){
    case NONE:
        break;
    case PING:
        ping_react(data);
        break;
    case LS:
        ls_react(data);
        break;
    case CAT:
        cat_react(data);
        break;
    }
}
