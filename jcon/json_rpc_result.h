#pragma once

#include "jcon.h"

#include <QString>
#include <QVariant>

namespace jcon {

class JCON_API JsonRpcResult
{
public:
    virtual ~JsonRpcResult() {}

    operator bool() const { return isSuccess(); }

    virtual bool isSuccess() const = 0;
    virtual QVariant result() const = 0;
    virtual QString toString() const = 0;
};

class JCON_API JsonRpcSuccess : public JsonRpcResult
{
public:
    JsonRpcSuccess(QVariant result):
        m_result(result)
    {
    }

    bool isSuccess() const override { return true; }
    QVariant result() const override { return m_result; }
    QString toString() const override { return m_result.toString(); }

private:
    QVariant m_result;
};

class JCON_API JsonRpcError : public JsonRpcResult
{
public:
    enum ErrorCodes {
        EC_ParseError     = -32700,
        EC_InvalidRequest = -32600,
        EC_MethodNotFound = -32601,
        EC_InvalidParams  = -32602,
        EC_InternalError  = -32603
    };

    JsonRpcError(int code, const QString& message = "", const QVariant& data = QVariant()):
        m_code(code),
        m_message(message),
        m_data(data)
    {
    }

    bool isSuccess() const override { return false; }
    QVariant result() const override { return QVariant(); }
    QString toString() const override{ return QString("%1 (%2)").arg(message()).arg(code());}

private:
    int code() const { return m_code; }
    QString message() const { return m_message; }
    QVariant data() const { return m_data; }

    int m_code;
    QString m_message;
    QVariant m_data;
};

}
