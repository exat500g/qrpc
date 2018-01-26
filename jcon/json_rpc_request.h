#pragma once

#include "jcon.h"

#include <QDateTime>
#include <QObject>
#include <QUuid>

namespace jcon {

class JCON_API JsonRpcRequest : public QObject
{
    Q_OBJECT

public:
    JsonRpcRequest(QObject* parent,
                   RequestId id,
                   QDateTime timestamp = QDateTime::currentDateTime());
    virtual ~JsonRpcRequest();

    RequestId id() const;

signals:
    void result(const QVariant& result);
    void error(int code, const QString& message, const QVariant& data);

private:
    RequestId m_id;
    QDateTime m_timestamp;
};

}
