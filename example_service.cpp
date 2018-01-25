#include "example_service.h"

#include <QDebug>
#include <QtGlobal>


int ExampleService::getRandomInt(int limit)
{
    return qrand() % limit;
}

QString ExampleService::printMessage(const QString& msg)
{
    qDebug().noquote() << QString("-> printMessage: '%1'").arg(msg);
    return QString("Return: '%1'").arg(msg);
}

void ExampleService::printNotification(const QString &msg) {
    qDebug().noquote() << QString("-> printNotification: '%1'").arg(msg);
}
void ExampleService::printU64(qulonglong num){
    qDebug().noquote() << QString("-> printU64: '%1'").arg(num);
}
