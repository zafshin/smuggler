#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QTcpSocket>

class TcpSocket : public QTcpSocket
{
    Q_OBJECT

private:
    struct StatusType
    {
        unsigned long bytesDownload=0;
        unsigned long bytesUpload=0;
        unsigned long limitDownload=0;
        unsigned long limitUpload=0;
        unsigned int sessions=0;
        QString name="";
        QString key="";
    };
public:
    StatusType Status;
};

#endif // TCPSOCKET_H
