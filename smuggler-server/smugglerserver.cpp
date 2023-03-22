#include "smugglerserver.h"
#include "portforwarder.h"
#include <QSslConfiguration>
#include <QSslCipher>


smugglerServer::smugglerServer(QObject *parent,QVector<unsigned int> port, QString portl)
{
    foreach (auto item, port) {
       QTcpServer* s = new QTcpServer();
       server.push_back(s);
       connect(s, &QTcpServer::newConnection, this, &smugglerServer::newConnectionReq);
       if (s->listen(QHostAddress::Any, item)) {
            std::cout << "Listening: " << item <<"\n";
       }
    }

    portll = portl;
//    sslConfiguration = new QSslConfiguration ;
    // Load the SSL certificate and key
//    QFile certFile("server.crt");
//    certFile.open(QIODevice::ReadOnly);
//    certificate = new QSslCertificate(&certFile, QSsl::Pem);
//    certFile.close();

//    QFile keyFile("server.key");
//    keyFile.open(QIODevice::ReadOnly);
//    privateKey = new QSslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
//    keyFile.close();
//    std::cout << "Keys are loaded" <<"\n";
//    sslConfiguration->setLocalCertificate(*certificate);
//    sslConfiguration->setPrivateKey(*privateKey);
//    QList<QSslCipher> ciphers;
//    ciphers << QSslCipher("TLS_AES_128_GCM_SHA256");
//    sslConfiguration->setCiphers(ciphers);
    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&smugglerServer::timerTick);
    updateFromJason();
    timer->setSingleShot(false);
    timer->setInterval(10000);
    timer->start();
    std::cout << "Initialization is complete." <<"\n";
}

void smugglerServer::newConnectionReq(){
    QTcpServer* obj = qobject_cast<QTcpServer*>(sender());
    QTcpSocket* plainSocket = obj->nextPendingConnection();
//    QSslSocket* sslSocket = new QSslSocket();
//    sslSocket->setSocketDescriptor(plainSocket->socketDescriptor());
//    // Set the SSL certificate and key
//    sslSocket->setPeerVerifyMode(QSslSocket::VerifyNone);
//    sslSocket->setSslConfiguration(*sslConfiguration);
//    // Start the SSL handshake
//    sslSocket->startServerEncryption();

    QThread* newQThread = new QThread;
    std::cout << "New connection from: " << plainSocket->peerAddress().toString().toStdString() << "\n";
    portForwarder* newPF= new portForwarder(plainSocket,portll);
    newPF->moveToThread(newQThread);
    plainSocket->setParent(nullptr);
    plainSocket->moveToThread(newQThread);
    connect(newQThread, &QThread::started, newPF, &portForwarder::start);
    connect(newPF, &portForwarder::statsUpdate, this, &smugglerServer::updateStatus);
    connect(newPF, &portForwarder::ath, this, &smugglerServer::checkAthu);
    connect(newPF, &portForwarder::connectionClose, this, &smugglerServer::closeConnection);
    connect(this,  &smugglerServer::AthuOK, newPF,&portForwarder::ath);
    newQThread->start();
    //    Connections.push_back(socket);
    //    StatusType *st = new StatusType;
    //    st->Self = socket;
    //    socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 10000000);
    //    socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 10000000);
    //    socket->setSocketOption(QAbstractSocket::SocketOption::LowDelayOption,1);
    //    st->hashKey = QCryptographicHash::hash(st->key.toLocal8Bit(), QCryptographicHash::Sha256);
    //    Status.push_back(st);
    //    connect(socket,&QTcpSocket::connected ,[=](){this->connectionConnected(socket,st);});
    //    connect(socket,&QTcpSocket::disconnected ,[=](){this->connectionDisconnected(socket,st);});
    //    connect(socket,&QTcpSocket::readyRead ,[=](){this->connectionDataRx(socket,st);});
    //    Log("New Connection :" + socket->peerAddress().toString());
    //    st->Bypass.connectToHost(st->BypassIp,st->BypassPort);
    //    st->Bypass.setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 10000000);
    //    st->Bypass.setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 10000000);
    //    st->Bypass.setSocketOption(QAbstractSocket::SocketOption::LowDelayOption,1);
    //    connect(&st->Bypass,&QTcpSocket::connected ,[=](){this->BypassConnected(socket,st);});
    //    connect(&st->Bypass,&QTcpSocket::disconnected ,[=](){this->BypassDisconnected(socket,st);});
    //    connect(&st->Bypass,&QTcpSocket::readyRead ,[=](){this->BypassDataRx(socket,st);});
}

void smugglerServer::Log(QString l){
    l += "\r\n";
    std::cout << l.toStdString();
}


void smugglerServer::checkAthu(QString name){
    bool flag= false;
    QDate date = QDate::currentDate();
    portForwarder* obj = qobject_cast<portForwarder*>(sender());
    //std::cout << name.toStdString() << "\n";
    foreach (StatusType* item, Status) {
        if(item->hash == name){
            flag = true;
            if(item->sessions < item->limitSessions){

                if(item->ExpirationDate < date){
                    QMetaObject::invokeMethod(obj, "close", Qt::QueuedConnection);
                    std::cout << "Name: " << item->name.toStdString() << ", Authentication: Expired." << "\n";
                }else{
                    item->sessions += 1;
                    std::cout << "Name: " << item->name.toStdString() << ", Authentication: OK." << "\n";
                    QMetaObject::invokeMethod(obj, "athOk", Qt::QueuedConnection);
                }
            }else{
                QMetaObject::invokeMethod(obj, "close", Qt::QueuedConnection);
                std::cout << "Name: " << item->name.toStdString() << ", Authentication: session limit." << "\n";
            }
            break;
        }
    }
    if(!flag){
        foreach (dataBase* item, db){
            QByteArray hashValue = QCryptographicHash::hash(item->name.toLatin1(), QCryptographicHash::Sha1);

            if(hashValue == name){
                flag = true;
                StatusType* n = new StatusType;
                n->Download =item->Download;
                n->Upload =item->Upload;
                n->limitDownload =item->limitDownload;
                n->limitUpload =item->limitUpload;
                n->limitSessions =item->sessions;
                n->CreatedDate =item->CreatedDate;
                n->ExpirationDate =item->ExpirationDate;
                n->name =item->name;
                n->hash = QCryptographicHash::hash(item->name.toLatin1(), QCryptographicHash::Sha1);
                if(!(n->ExpirationDate < date)){
                    if(item->sessions > 0){
                        n->sessions += 1;
                        Status.push_back(n);
                        std::cout << "Name: " << n->name.toStdString() << ", Authentication: OK" << ".\n";
                        QMetaObject::invokeMethod(obj, "athOk", Qt::QueuedConnection);
                    }else
                    {
                        QMetaObject::invokeMethod(obj, "close", Qt::QueuedConnection);
                        std::cout << "Name: " << item->name.toStdString() << ", Authentication: session limit." << "\n";
                    }
                }else{
                    QMetaObject::invokeMethod(obj, "close", Qt::QueuedConnection);
                    std::cout << "Name: " << item->name.toStdString() << ", Authentication: Expired." << "\n";
                }
                break;
            }
        }
    }
    if(!flag){
        QMetaObject::invokeMethod(obj, "close", Qt::QueuedConnection);
        std::cout << "Name: Not find" << ", Authentication: fail" << ".\n";

    }
}

void smugglerServer::updateStatus(QString name, unsigned long download,unsigned long upload){
    portForwarder* obj = qobject_cast<portForwarder*>(sender());
    foreach (StatusType* item, Status) {
        if(item->hash == name){
            item->Download += download;
            item->Upload += upload;
            QDate date = QDate::currentDate();
            if(item->ExpirationDate < date){
                QMetaObject::invokeMethod(obj, "disconnectd", Qt::QueuedConnection);
                std::cout << "Name: " << item->name.toStdString() << ", Authentication: Expired." << "\n";
            }
            //std::cout << "Name: " << item->name.toStdString() << ", Status Update: download: "<< item->Download << " Kb update: " \
            //          << item->Upload << " kb"<< ".\n";
            break;
        }
    }
}

void smugglerServer::closeConnection(QString name,unsigned short error){
    portForwarder* obj = qobject_cast<portForwarder*>(sender());

    foreach (StatusType* item, Status) {
        if(item->hash == name){
            std::cout << "Name: " << item->name.toStdString() << ", Disconnected" << ".\n";
            if(error ==0){
                if(item->sessions >0)
                    item->sessions--;
                break;
            }
        }
    }
    QMetaObject::invokeMethod(obj, "deleteit", Qt::QueuedConnection);
    obj->disconnect();
}

void smugglerServer::saveToJason(){
    foreach (auto item, Status) {
        foreach (auto f, db) {
            if(f->name == item->name){
                f->Download = item->Download;
                f->Upload = item->Upload;
                f->connections = item->sessions;
            }

        }

    }
    QJsonArray jsonArray;
    foreach(dataBase* structElement, db) {

        QJsonObject jsonObject;
        jsonObject["name"] = structElement->name;
        jsonObject["Download"] = QString::number(structElement->Download);
        jsonObject["Upload"] = QString::number(structElement->Upload);
        jsonObject["limitDownload"] = QString::number(structElement->limitDownload);
        jsonObject["limitUpload"] = QString::number(structElement->limitUpload);
        jsonObject["sessions"] = QString::number(structElement->sessions);
        jsonObject["connections"] = QString::number(structElement->connections);
        jsonObject["ExpirationDate"] = structElement->ExpirationDate.toString("yyyy-MM-dd");
        jsonObject["CreatedDate"] = structElement->CreatedDate.toString("yyyy-MM-dd");
        jsonArray.append(jsonObject);
    }
    QJsonDocument jsonDoc(jsonArray);

    QByteArray jsonData = jsonDoc.toJson();
    QFile jsonFile("db.stats.json");
    if (jsonFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        jsonFile.write(jsonData);
        jsonFile.close();
    }
}
void smugglerServer::updateFromJason(){
    QFile jsonFile("db.json");
    if (jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QVector<dataBase*> newdb;
        QByteArray json = jsonFile.readAll();
        jsonFile.close();
        QJsonDocument jdoc = QJsonDocument::fromJson(json);
        QJsonArray jsonArray = jdoc.array(); // The array to read
        for (int i = 0; i < jsonArray.size(); i++) {
            QJsonObject jsonObject = jsonArray[i].toObject();
            dataBase* newd = new dataBase;
            newd->CreatedDate = QDate::fromString(jsonObject["CreatedDate"].toString(), "yyyy-MM-dd");
            newd->ExpirationDate = QDate::fromString(jsonObject["ExpirationDate"].toString(), "yyyy-MM-dd");
            newd->sessions = jsonObject["sessions"].toString().toUInt();
            newd->limitUpload = jsonObject["limitUpload"].toString().toUInt();
            newd->limitDownload = jsonObject["limitDownload"].toString().toUInt();
            newd->Upload = jsonObject["Upload"].toString().toUInt();
            newd->Download = jsonObject["Download"].toString().toUInt();
            newd->name = jsonObject["name"].toString();
            newdb.push_back(newd);
        }
        foreach (auto element, db) {
            delete element;
        }
        db.clear();
        db = newdb;
    }

    QFile jsonFiles("db.stats.json");
    if (jsonFiles.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QVector<dataBase*> newdb;
        QByteArray json = jsonFiles.readAll();
        jsonFiles.close();
        QJsonDocument jdoc = QJsonDocument::fromJson(json);
        QJsonArray jsonArray = jdoc.array(); // The array to read
        for (int i = 0; i < jsonArray.size(); i++) {
            QJsonObject jsonObject = jsonArray[i].toObject();
            dataBase* newd = new dataBase;
            newd->Upload = jsonObject["Upload"].toString().toUInt();
            newd->Download = jsonObject["Download"].toString().toUInt();
            newd->name = jsonObject["name"].toString();
            newdb.push_back(newd);
        }
        foreach (auto element, db) {
            foreach (auto el,newdb) {
                if(element->name == el->name){
                    element->Download = el->Download;
                    element->Upload = el->Upload;
                    break;
                }
            }
        }
    }
    foreach (auto element, db) {
        foreach (auto statusElement, Status) {
            if(statusElement->name == element->name){
                statusElement->CreatedDate = element->CreatedDate;
                statusElement->CreatedDate = element->CreatedDate;
                statusElement->ExpirationDate = element->ExpirationDate;
                statusElement->limitDownload = element->limitDownload;
                statusElement->limitUpload = element->limitUpload;
            }
        }
    }

}
void smugglerServer::updateDBJason(){
    QFile jsonFile("db_new.json");
    if(jsonFile.exists()){
        if (jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = jsonFile.readAll();
            QFile jsonFileo("db.json");
            if (jsonFileo.open(QIODevice::WriteOnly | QIODevice::Text)) {
                jsonFileo.write(data);
                jsonFileo.close();
            }
            jsonFile.close();
            jsonFile.remove();
        }
        updateFromJason();
    }
}
void smugglerServer::timerTick(){
    updateDBJason();
    saveToJason();

}
