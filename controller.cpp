#include <QApplication>
#include "controller.h"

/*!
 * ... coucou c'est mon commentaire hyper pratique qui sert à rien ...
 */
Controller::Controller(QObject *parent)
{

    this->setParent(parent);
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("OST server"),QWebSocketServer::NonSecureMode, this);
    if (m_pWebSocketServer->listen(QHostAddress::Any, 9624)) {
       connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this, &Controller::onNewConnection);
       connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &Controller::closed);
    }
    IDLog("OST server listening\n");

    indiclient = new MyClient(this);
}

Controller::~Controller()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}


void Controller::sendmessage(QString message)
{
    for (int i=0;i<m_clients.size();i++) {
        QWebSocket *pClient = m_clients[i];
        if (pClient) pClient->sendTextMessage(message);
    }

}
void Controller::sendbinary(QByteArray *data)
{
    for (int i=0;i<m_clients.size();i++) {
        QWebSocket *pClient = m_clients[i];
        if (pClient) pClient->sendBinaryMessage(*data);
    }
}


void Controller::onNewConnection()
{
    IDLog("New client connection\n");
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    connect(pSocket, &QWebSocket::textMessageReceived, this, &Controller::processTextMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &Controller::socketDisconnected);
    m_clients << pSocket;
}


void Controller::processTextMessage(QString message)
{
    IDLog("OST server received text message %s\n",message.toStdString().c_str() );
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8()); // garder
    QJsonObject  obj = jsonResponse.object(); // garder

    if ( (obj["message"].toString()=="shoot")
      || (message=="shoot") )
    {
        if (indiclient->isServerConnected())
        {
            //processShoot();
        } else {
            IDLog("Indi server not connected\n");
        }
    }

    if ( (obj["message"].toString()=="connectindi")
      || (message=="connectindi") )
    {
        if (!(indiclient->isServerConnected()))
        {
            indiclient->connectIndi();
        } else {
            //IDLog("Indi server not connected\n");
        }
    }

    if ( (obj["message"].toString()=="connectdevices")
      || (message=="connectdevices") )
    {
        if (indiclient->isServerConnected())
        {
            indiclient->connectAllDevices();
        } else {
            IDLog("Indi server not connected\n");
        }
    }

    if ( (obj["message"].toString()=="disconnectdevices")
      || (message=="disconnectdevices") )
    {
        if (indiclient->isServerConnected())
        {
            indiclient->disconnectAllDevices();
        } else {
            IDLog("Indi server not connected\n");
        }
    }
    if ( (obj["message"].toString()=="loadconfs")
      || (message=="loadconfs") )
    {
        if (indiclient->isServerConnected())
        {
            indiclient->loadDevicesConfs();
        } else {
            IDLog("Indi server not connected\n");
        }
    }
}

void Controller::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    IDLog("OST server received binary message\n");

    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}

void Controller::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    IDLog("OST client disconnected\n");
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}



