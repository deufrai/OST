#ifndef OSTClientGEN_h_
#define OSTClientGEN_h_
#pragma once

//QT Includes
//QT Includes
#include <QDir>
#include <QThread>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QRect>
#include <QPointer>
#include <QtNetwork>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <QJsonObject>
#include <QJsonDocument>
#include <OSTClient.h>

class OSTClientGEN : public OSTClient
{
  public:
    OSTClientGEN(QObject *parent = Q_NULLPTR);
    virtual ~OSTClientGEN();

private:

signals:


};

#endif
