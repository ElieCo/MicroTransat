#include "backend.h"
#include "QDebug"
#include "QFile"
#include "QDir"

BackEnd::BackEnd(QObject *parent)
  : QObject(parent)
{
}

QStringList BackEnd::titles()
{
    return m_titles;
}
