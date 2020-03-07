#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QString>
#include <QMap>

#include "comm_manager.h"

class BackEnd : public QObject
{
    Q_OBJECT

public:
    explicit BackEnd(QObject *parent = nullptr);

    Q_PROPERTY(QStringList titles READ titles NOTIFY titlesChanged)

    QStringList titles();

private:
    CommManager cm;
    QStringList m_titles;

signals:
    void titlesChanged();

};

#endif // BACKEND_H
