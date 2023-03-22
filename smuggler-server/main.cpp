#include <QCoreApplication>
#include "smugglerserver.h"
//#include <qrsaencryption.h>
#include <iostream>
#include <QFile>
smugglerServer* b;
int main(int argc, char *argv[])
{

    QCoreApplication a(argc, argv);
    QStringList args = a.arguments();
    QString forward;
    QVector<unsigned int> ports;
    for(int i = 0; i<args.length();i++){
        if(i>0){
            if(i == 1){
                std::cout << "Forwarding to " << args[i].toStdString() << ".\n";
                forward = args[i];
            }else{
                ports.push_back(args[i].toUInt());
            }
        }
    }
    if(ports.size() > 0){
         b = new smugglerServer(nullptr,ports,forward);
    }
    else
        return 0;
    return a.exec();
}
