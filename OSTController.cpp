#include "OSTController.h"
#include "OSTClient.h"
#include "OSTClientGEN.h"
#include "OSTClientPAN.h"
#include "OSTClientCTL.h"
#include "OSTClientFOC.h"
#include <vector>
#include <string>
#include "OSTParser.h"
#include <unistd.h>


OSTController::OSTController(QObject *parent)
{
    //OSTClientGEN *MyOSTClientGEN = new OSTClientGEN(parent);

    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("OST server"),QWebSocketServer::NonSecureMode, this);
    if (m_pWebSocketServer->listen(QHostAddress::Any, 9624)) {
       IDLog("OST server listening on port %i\n",9624);
       connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this, &OSTController::onNewConnection);
       connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &OSTController::closed);
    }
    properties = QJsonObject();
    QJsonObject prop;
    QJsonArray allprops;
    /*QFile file;
    QString val,filetoread;
    filetoread=PropertiesFolder+"/";
    file.setFileName(PropertiesFolder+"/OST.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    //qWarning() << val;
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
    properties = d.object();*/

    //connect(MyOSTClientPAN.get(), &OSTClientPAN::sendmessage, this, &OSTController::sendjson );
    MyOSTClientCTL.reset(new OSTClientCTL(parent));
    MyOSTClientGEN.reset(new OSTClientGEN(parent));
    MyOSTClientPAN.reset(new OSTClientPAN(parent));
    MyOSTClientFOC.reset(new OSTClientFOC(parent));
    MyOSTClientGUI.reset(new OSTClientGUI(parent));
    connect(MyOSTClientPAN.get(),&OSTClientPAN::sendjson, this, &OSTController::sendjson );
    connect(MyOSTClientCTL.get(),&OSTClientCTL::sendjson, this, &OSTController::sendjson );
    connect(MyOSTClientGEN.get(),&OSTClientGEN::sendjson, this, &OSTController::sendjson );
    connect(MyOSTClientFOC.get(),&OSTClientFOC::sendjson, this, &OSTController::sendjson );
    connect(MyOSTClientGUI.get(),&OSTClientGUI::sendjson, this, &OSTController::sendjson );
    connect(MyOSTClientGUI.get(),&OSTClientGUI::sendbinary, this, &OSTController::sendbinary);


}

OSTController::~OSTController()
{
    IDLog("OSTController delete\n");
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
    //delete MyOSTClientFOC;
    //delete MyOSTClientGEN;
}


void OSTController::sendmessage(QString message)
{
    for (int i=0;i<m_clients.size();i++) {
        QWebSocket *pClient = m_clients[i];
        if (pClient) pClient->sendTextMessage(message);
    }

}
void OSTController::sendbinary(QByteArray *data)
{


    for (int i=0;i<m_clients.size();i++) {
        QWebSocket *pClient = m_clients[i];
        if (pClient) pClient->sendBinaryMessage(*data);
    }
}


void OSTController::sendjson(QJsonObject json)
{
    QJsonDocument jsondoc;
    jsondoc.setObject(json);
    //QString strJson(jsondoc.toJson(QJsonDocument::Indented)); // version lisible
    QString strJson(jsondoc.toJson(QJsonDocument::Compact)); // version compactée
    sendmessage(strJson);




}



void OSTController::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    IDLog("OST server new client connection\n");

    connect(pSocket, &QWebSocket::textMessageReceived, this, &OSTController::processTextMessage);
    connect(pSocket, &QWebSocket::textMessageReceived, MyOSTClientCTL.get(), &OSTClientCTL::processTextMessage);
    connect(pSocket, &QWebSocket::textMessageReceived, MyOSTClientGEN.get(), &OSTClientGEN::processTextMessage);
    connect(pSocket, &QWebSocket::textMessageReceived, MyOSTClientPAN.get(), &OSTClientPAN::processTextMessage);
    connect(pSocket, &QWebSocket::textMessageReceived, MyOSTClientFOC.get(), &OSTClientFOC::processTextMessage);
    connect(pSocket, &QWebSocket::textMessageReceived, MyOSTClientGUI.get(), &OSTClientGUI::processTextMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &OSTController::socketDisconnected);

    m_clients << pSocket;
    //emitall();
}


void OSTController::processTextMessage(QString message)
{

    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8()); // garder
    QJsonObject  obj = jsonResponse.object(); // garder
    //IDLog("message received-------------------------------------\n");
    //IDLog("%s\n",QJsonDocument(obj).toJson(QJsonDocument::Indented).toStdString().c_str());
    //IDLog("message received end --------------------------------\n");
    //sendjson(MyOSTClientPAN->getmodule("OSTClientPAN"));
    //MyOSTClientPAN->updateproperty("OSTClientPAN","CCD Simulator","Main Control","CONNECTION");
    //connect(MyOSTClientPAN.get(), &OSTClientPAN::sendmessage, this, &OSTController::sendjson );

    if (obj["message"].toString()=="readall") {
        QJsonArray mods;
        QJsonObject mod,tosend;
        mod=MyOSTClientCTL->getmodule("OSTClientCTL");
        mods.append(mod);
        mod=MyOSTClientPAN->getmodule("OSTClientPAN");
        mods.append(mod);
        mod=MyOSTClientGEN->getmodule("OSTClientGEN");
        mods.append(mod);
        mod=MyOSTClientFOC->getmodule("OSTClientFOC");
        mods.append(mod);
        mod=MyOSTClientGUI->getmodule("OSTClientGUI");
        mods.append(mod);
        tosend["modules"]=mods;
        sendjson(tosend);

        /*
        QByteArray data;
        QString toto = "toto";
        data =toto.toUtf8();
        sendbinary(&data);
        */
        /*QImage *ii = new QImage ("/home/gilles/OST/toto.jpeg");
        QByteArray ba;
        QBuffer buffer(&ba);
        ii->save(&buffer, "JPEG");
        sendbinary(&ba);*/
    }
    if (obj["message"].toString()=="readmodule") {
        if (obj["module"].toString()=="OSTClientPAN") {
            MyOSTClientPAN->updatemodule("OSTClientPAN");
        }
        if (obj["module"].toString()=="OSTClientCTL") {
            MyOSTClientCTL->updatemodule("OSTClientCTL");
        }
        if (obj["module"].toString()=="OSTClientGEN") {
            MyOSTClientGEN->updatemodule("OSTClientGEN");
        }
        if (obj["module"].toString()=="OSTClientFOC") {
            MyOSTClientFOC->updatemodule("OSTClientFOC");
        }
        if (obj["module"].toString()=="OSTClientGUI") {
            MyOSTClientGUI->updatemodule("OSTClientGUI");
        }
    }
    if (obj["message"].toString()=="readcategory") {
        if (obj["module"].toString()=="OSTClientPAN") {
            MyOSTClientPAN->updatecategory("OSTClientPAN",obj["category"].toString());
        }
    }
    if (obj["message"].toString()=="readgroup") {
        if (obj["module"].toString()=="OSTClientPAN") {
            MyOSTClientPAN->updategroup("OSTClientPAN",obj["category"].toString(),obj["group"].toString());
        }
    }
    if (obj["message"].toString()=="readproperty") {
        if (obj["module"].toString()=="OSTClientPAN") {
            MyOSTClientPAN->updateproperty("OSTClientPAN",obj["category"].toString(),obj["group"].toString(),obj["property"].toString());
        }
    }
    if (obj["message"].toString()=="readproperty") {
        if (obj["module"].toString()=="OSTClientGUI") {
            MyOSTClientGUI->updateproperty("OSTClientGUI",obj["category"].toString(),obj["group"].toString(),obj["property"].toString());
        }
    }
}

void OSTController::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    IDLog("OST server received binary message\n");

    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}

void OSTController::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    IDLog("OST server disconnect\n");
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void OSTController::emitnewprop(INDI::Property *property) {
    //IDLog("emit new prop %s\n",property->getName());
}
void OSTController::emitnewtext(ITextVectorProperty *prop) {
    //sendjson(tINDItoJSON(prop));
}
void OSTController::emitnewnumber(INumberVectorProperty *prop) {
    //sendjson(nINDItoJSON(prop));
}
void OSTController::emitnewswitch(ISwitchVectorProperty *prop) {
    //sendjson(sINDItoJSON(prop));
}
void OSTController::emitnewlight(ILightVectorProperty *prop) {
    //sendjson(lINDItoJSON(prop));
}
void OSTController::emitnewdevice(INDI::BaseDevice *d) {
    //sendjson(dINDItoJSON(d));
}
