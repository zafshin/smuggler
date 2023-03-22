#include "portforwarder.h"
#include <iostream>

portForwarder::portForwarder(QTcpSocket *clientSocket, QString port, QObject *parent)
    : QObject(parent), m_clientSocket(clientSocket), portl(port)
{

}

void portForwarder::start(){
    // Connect to the desired destination
    ip = m_clientSocket->peerAddress().toString();
    timer = new QTimer;
    timer->moveToThread(QThread::currentThread());
    timer->setSingleShot(false);
    timer->setInterval(8000);
    connect(timer,&QTimer::timeout,this,&portForwarder::timerTik);
    timer->start();
    destinationSocket = new QTcpSocket;
    m_clientSocket->setSocketOption(QAbstractSocket::SocketOption::LowDelayOption,1);
    destinationSocket->setSocketOption(QAbstractSocket::SocketOption::LowDelayOption,1);
    destinationSocket->connectToHost("127.0.0.1", portl.toInt());
    //std::cout << "Connected to:"<< m_clientSocket->peerAddress().toString().toStdString() << " .\n";
    // Set up a bidirectional connection between the client and destination
    connect(m_clientSocket, &QSslSocket::readyRead, this, &portForwarder::relayDataFromClient);
    connect(m_clientSocket, &QSslSocket::disconnected, this, &portForwarder::disconnectm);
    connect(destinationSocket, &QTcpSocket::readyRead, this, &portForwarder::relayDataFromDestination);
    connect(destinationSocket, &QTcpSocket::disconnected, this, &portForwarder::disconnectd);
}

void portForwarder::relayDataFromClient(){

    // Read data from the client and write it to the destination

    if(!athStatus){
        while (m_clientSocket->canReadLine())
        {
            QByteArray data = m_clientSocket->readLine();
            //std::cout << "From client " << data.toStdString() << " .\n";
            data = QByteArray::fromBase64(data);
            name = data;
            emit ath(data);
        }
    }else{
        QByteArray data = m_clientSocket->readAll();
        if(data.size()>0){
            //std::cout << "From server " << data.size() << " .\n";
            if(uploadBytes > 1000){
                int temp = uploadBytes / 1000;
                uploadKBytes += temp;
                uploadBytes -= temp * 1000;
            }
            uploadBytes += data.size();
            destinationSocket->write(data);
            destinationSocket->flush();
        }
    }
}

void portForwarder::relayDataFromDestination(){
    // Read data from the destination and encrypt it using the private key
    QByteArray data = destinationSocket->readAll();
    if(data.size()>0){
        if(downloadBytes > 1000){
            int temp = downloadBytes / 1000;
            downloadKBytes += temp;
            downloadBytes -= temp * 1000;
        }
        //std::cout << "From server " << data.size() << " .\n";
        // Write the encrypted data to the client
        downloadBytes += data.size();
        m_clientSocket->write(data);
        m_clientSocket->flush();
    }
}

void portForwarder::disconnectm(){
    // Disconnect both sockets and close the thread
    m_clientSocket->disconnectFromHost();
    destinationSocket->disconnectFromHost();
    emit finished();
    emit connectionClose(name,er);
    timer->stop();
    timerTik();
}
void portForwarder::disconnectd(){
    // Disconnect both sockets and close the thread
//    std::cout << "Disconnected from:"<< ip.toStdString() <<"\n"<<"Total download: " << \
//              downloadKBytes<< " KByte, Total upload: " << uploadKBytes<< " KByte.\n";
    m_clientSocket->disconnectFromHost();
}

void portForwarder::close(){
    er = 1;
    portForwarder::disconnectd();
}

void portForwarder::athOk(){
    athStatus = true;
    //std::cout << "Traffic Allowed"<<"\n";
    m_clientSocket->write("OK\n");
    m_clientSocket->flush();
}
void portForwarder::timerTik(){
    emit statsUpdate(name,downloadKBytes,uploadKBytes);
    downloadKBytes = 0;
    uploadKBytes = 0;
}
void portForwarder::deleteit(){
   timer->disconnect();
   this->deleteLater();
   this->disconnect();
}
