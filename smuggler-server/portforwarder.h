#ifndef PORTFORWARDER_H
#define PORTFORWARDER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QObject>
#include <QSslSocket>
#include <QByteArray>
#include <QTimer>
class portForwarder : public QObject
{
    Q_OBJECT
public:
    portForwarder(QTcpSocket *clientSocket,QString port, QObject *parent = nullptr);
public slots:
    void start();
signals:
    void finished();
    void statsUpdate(QString name,unsigned long download,unsigned long upload);
    void ath(QString name);
    void connectionClose(QString name,unsigned short error);
public slots:
    void relayDataFromClient();
    void relayDataFromDestination();
    void athOk();
    void disconnectm();
    void disconnectd();
    void timerTik();
    void close();
    void deleteit();
private:
//    QRSAEncryption* e;
    bool athStatus = false;
    QTimer* timer;
    QByteArray pub;
    QByteArray priv;
    QString name;
    unsigned short er=0;
    QString ip;
    unsigned int downloadBytes=0;
    unsigned int uploadBytes=0;
    unsigned int downloadKBytes=0;
    unsigned int uploadKBytes=0;
    QTcpSocket *m_clientSocket;
    QTcpSocket *destinationSocket;
    QString portl;
};

#endif // PORTFORWARDER_H
