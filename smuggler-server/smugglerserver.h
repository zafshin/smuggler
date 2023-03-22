#ifndef SMUGGLERSERVER_H
#define SMUGGLERSERVER_H
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>
#include <QVector>
#include <QString>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDataStream>
#include <QDate>
#include <iostream>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <qdebug.h>
#include <QFile>
#include <QTimer>
#include <QSslCertificate>
#include <QSslKey>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QVector>


class smugglerServer : public QObject
{
    Q_OBJECT

private:
    struct StatusType
    {
        unsigned long long Download=0;
        unsigned long long Upload=0;
        unsigned long limitDownload=0;
        unsigned long limitUpload=0;
        unsigned int sessions=0;
        unsigned int limitSessions=2;
        QDate CreatedDate;
        QDate ExpirationDate;
        QString hash="";
        QString name="";
    };

    struct dataBase
    {
        unsigned long long Download=0;
        unsigned long long Upload=0;
        unsigned long limitDownload=0;
        unsigned long limitUpload=0;
        QDate CreatedDate;
        QDate ExpirationDate;
        unsigned int sessions=0;
        unsigned int connections=0;
        QString name="";
    };

    QVector<QTcpServer*> server;
    QVector<QTcpSocket*> Connections;
    QVector<StatusType*> Status;
    QVector<dataBase*> db;
    QList<QString> log;
    unsigned short er = 0;
    void Log(QString l);
    void saveToJason();
    void updateFromJason();
    void updateDBJason();
    QTimer* timer;
    QString portll;
    QSslKey* privateKey;
    QSslCertificate* certificate;
    QSslConfiguration* sslConfiguration;
signals:
    void AthuOK(QString name);
public:
    smugglerServer(QObject *parent ,QVector<unsigned int> port, QString portl = 0);
public slots:
    void newConnectionReq();
    void closeConnection(QString name,unsigned short error);
    void timerTick();
    void checkAthu(QString name);
    void updateStatus(QString name, unsigned long download,unsigned long upload);
};

#endif // SMUGGLERSERVER_H
