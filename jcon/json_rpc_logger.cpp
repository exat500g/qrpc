#include "json_rpc_logger.h"
#include <QDebug>

namespace jcon {

JsonRpcLogger::JsonRpcLogger()
{
}

JsonRpcLogger::~JsonRpcLogger()
{
}

void JsonRpcLogger::logDebug(const QString& message)
{
    qDebug().noquote() << message;
}

void JsonRpcLogger::logInfo(const QString& message)
{
    qDebug().noquote() << message;
}

void JsonRpcLogger::logWarning(const QString& message)
{
    qDebug().noquote() << message;
}

void JsonRpcLogger::logError(const QString& message)
{
    qDebug().noquote() << message;
}

}


