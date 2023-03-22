#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QVector>
#include <iostream>
#include <QFile>
#include <QThread>
#include <QDate>
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
bool change =false;
QVector<dataBase*> db;
void updateFromJason(){
    QFile jsonFile("db.stats.json");
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
            newd->connections = jsonObject["connections"].toString().toUInt();
            newd->name = jsonObject["name"].toString();
            newdb.push_back(newd);
        }
        db = newdb;
    }
}
void saveToJason(){
    QJsonArray jsonArray;
    foreach(dataBase* structElement, db) {
        QJsonObject jsonObject;
        jsonObject["name"] = structElement->name;
        jsonObject["Download"] = QString::number(structElement->Download);
        jsonObject["Upload"] = QString::number(structElement->Upload);
        jsonObject["limitDownload"] = QString::number(structElement->limitDownload);
        jsonObject["limitUpload"] = QString::number(structElement->limitUpload);
        jsonObject["sessions"] = QString::number(structElement->sessions);
        jsonObject["ExpirationDate"] = structElement->ExpirationDate.toString("yyyy-MM-dd");
        jsonObject["CreatedDate"] = structElement->CreatedDate.toString("yyyy-MM-dd");
        jsonArray.append(jsonObject);

    }
    QJsonDocument jsonDoc(jsonArray);

    QByteArray jsonData = jsonDoc.toJson();
    QFile jsonFile("db_new.json");
    int i = 0;
    while(jsonFile.exists()){
        std::cout << "Wait for server to update...\n";
        QThread::msleep(1000);
        i++;
        if(i > 10){
            std::cout << "Error\n";
            qApp->exit(1);
        }
    }
    if (jsonFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        jsonFile.write(jsonData);
        jsonFile.close();
    }
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    updateFromJason();
    QJsonArray jsonArray;
    std::string inp;
    std::cout << "Do you want to see the users? (y/n)\n";
    std::cin >> inp;
    if(QString::fromStdString(inp).toLower()=="y"){
        foreach(auto item, db){

            std::cout << "Name: " << item->name.toStdString() << " ,Connected: " << item->connections << " ,Download: " << item->Download << "KB ,Upload: " << item->Upload << "KB ,Sessions: " << item->sessions << " ,Expire in: " << QDate::currentDate().daysTo(item->ExpirationDate) << ".\n";
        }
    }
    QFile jsonFile("db_new.json");
    if(jsonFile.exists())
        return 0;
    while(1){
        std::cout << "Do you want to add user? (y/n)\n";
        std::string input;
        std::cin >> input;
        QString name, session, Download, Upload, limitUpload \
                , limitDownload,CreatedDate,ExpirationDate;
        if(QString::fromStdString(input).toLower()=="y"){
            change = true;
            std::cout << "Enter the Name:? (xxxx#1234)\n";
            std::cin >> input;
            name = QString::fromStdString(input);
            std::cout << "Enter the sessions:? (1)\n";
            std::cin >> input;
            session = QString::fromStdString(input);
            std::cout << "Enter the Time:? (30)\n";
            std::cin >> input;
            QDate n = QDate::currentDate().addDays(QString::fromStdString(input).toInt());
            CreatedDate = QDate::currentDate().toString("yyyy-MM-dd");
            ExpirationDate = n.toString("yyyy-MM-dd");
            bool find =false;
            foreach (auto element, db) {
                if(element->name == name){
                    find = true;
                    element->sessions = session.toInt();
                    element->ExpirationDate = QDate::fromString(ExpirationDate, "yyyy-MM-dd");
                }
            }
            if(!find){
                dataBase* newd = new dataBase;
                newd->name = name;
                newd->CreatedDate = QDate::fromString(CreatedDate, "yyyy-MM-dd");
                newd->ExpirationDate = QDate::fromString(ExpirationDate, "yyyy-MM-dd");
                newd->sessions = session.toInt();
                db.push_back(newd);
            }
        }else
            break;

    }
    while(1){
        std::string input;
        std::cout << "Do you want to delete user? (y/n)\n";
        std::cin >> input;

        if(QString::fromStdString(input).toLower()=="y"){
            std::cout << "Enter the Name:? (xxxx#1234)\n";
            std::cin >> input;
            for (int i = 0;i<db.length();i++) {
                if(db[i]->name == QString::fromStdString(input)){
                    db.remove(i);
                    std::cout << "User has been deleted.\n";
                    change = true;
                }
            }
        }else{
            break;
        }

    }
    if(change)
        saveToJason();
    return 0;
}
