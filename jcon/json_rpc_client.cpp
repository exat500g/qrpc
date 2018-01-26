#include "json_rpc_client.h"
#include "json_rpc_logger.h"
#include "json_rpc_result.h"
#include "jcon_assert.h"
#include "string_util.h"

#include <QUuid>
#include <QCoreApplication>

#include <memory>

namespace jcon {

const QString JsonRpcClient::InvalidRequestId = "";

JsonRpcClient::JsonRpcClient(std::shared_ptr<JsonRpcSocket> socket,
                             QObject* parent,
                             std::shared_ptr<JsonRpcLogger> logger,
                             int call_timeout_ms)
    : QObject(parent)
    , m_logger(logger)
    , m_call_timeout_ms(call_timeout_ms)
    , m_outstanding_request_count(0)
{
    if (!m_logger) {
        m_logger = std::make_shared<JsonRpcLogger>();
    }

    m_endpoint = std::make_shared<JsonRpcEndpoint>(socket, m_logger, this);

    connect(m_endpoint.get(), &JsonRpcEndpoint::socketConnected,
            this, &JsonRpcClient::socketConnected);

    connect(m_endpoint.get(), &JsonRpcEndpoint::socketDisconnected,
            this, &JsonRpcClient::socketDisconnected);

    connect(m_endpoint.get(), &JsonRpcEndpoint::socketError,
            this, &JsonRpcClient::socketError);

    connect(m_endpoint.get(), &JsonRpcEndpoint::jsonObjectReceived,
            this, &JsonRpcClient::jsonResponseReceived);
}

JsonRpcClient::~JsonRpcClient()
{
    disconnectFromServer();
}

std::shared_ptr<JsonRpcResult>
JsonRpcClient::waitForSyncCallbacks(const JsonRpcRequest* request)
{
    connect(request, &JsonRpcRequest::result,
            [this, id = request->id()](const QVariant& result) {
                m_logger->logDebug(
                    QString("Received success response to synchronous "
                            "RPC call (ID: %1)").arg(id.toString()));

                m_results[id] = std::make_shared<JsonRpcSuccess>(result);
            });

    connect(request, &JsonRpcRequest::error,
            [this, id = request->id()](int code,
                                       const QString& message,
                                       const QVariant& data)
            {
                m_logger->logError(
                    QString("Received error response to synchronous "
                            "RPC call (ID: %1)").arg(id.toString()));

                m_results[id] =
                    std::make_shared<JsonRpcError>(code, message, data);
            });

    QTime timer;
    timer.start();
    while (m_outstanding_requests.contains(request->id()) &&
           timer.elapsed() < m_call_timeout_ms)
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    if (m_results.contains(request->id())) {
        auto res = m_results[request->id()];
        m_results.remove(request->id());
        return res;
    } else {
        return std::make_shared<JsonRpcError>(
            JsonRpcError::EC_InternalError,
            "RPC call timed out"
        );
    }
}

std::shared_ptr<JsonRpcResult>
JsonRpcClient::callExpandArgs(const QString& method, const QVariantList& args)
{
    auto req = doCallExpandArgs(method, false, args);
    return waitForSyncCallbacks(req.get());
}

std::shared_ptr<JsonRpcRequest>
JsonRpcClient::callAsyncExpandArgs(const QString& method,
                                   const QVariantList& args)
{
    return doCallExpandArgs(method, true, args);
}

std::shared_ptr<JsonRpcRequest>
JsonRpcClient::doCallExpandArgs(const QString& method,
                                bool async,
                                const QVariantList& args)
{
    std::shared_ptr<JsonRpcRequest> request;
    QJsonObject req_json_obj;
    std::tie(request, req_json_obj) = prepareCall(method);

    if (!args.empty()) {
        req_json_obj["params"] = QJsonArray::fromVariantList(args);
    }

    m_logger->logInfo(formatLogMessage(method, args, async, request->id()));
    m_endpoint->send(QJsonDocument(req_json_obj));

    return request;
}

int JsonRpcClient::outstandingRequestCount() const
{
    return m_outstanding_request_count;
}

void JsonRpcClient::verifyConnected(const QString& method)
{
    if (!isConnected()) {
        auto msg = QString("cannot call RPC method (%1) when not connected")
            .arg(method);
        m_logger->logError(msg);
        throw std::runtime_error(msg.toStdString());
    }
}

std::pair<std::shared_ptr<JsonRpcRequest>, QJsonObject>
JsonRpcClient::prepareCall(const QString& method)
{
    RequestId id=QUuid::createUuid();
    std::shared_ptr<JsonRpcRequest> request = std::make_shared<JsonRpcRequest>(this, id);
    m_outstanding_requests[id] = request;
    ++m_outstanding_request_count;
    QJsonObject req_json_obj = createRequestJsonObject(method, id);
    return std::make_pair(request, req_json_obj);
}

QJsonObject JsonRpcClient::createRequestJsonObject(const QString& method,
                                                   const RequestId &id)
{
    return QJsonObject {
        { "method", method },
        { "id", id.toString() }
    };
    QJsonObject obj;
}

bool JsonRpcClient::connectToServer(const QString& host, int port)
{
    if (!m_endpoint->connectToHost(host, port)) {
        return false;
    }
    return true;
}

bool JsonRpcClient::connectToServer(const QUrl& url)
{
    if (!m_endpoint->connectToUrl(url)) {
        return false;
    }
    return true;
}

void JsonRpcClient::disconnectFromServer()
{
    m_endpoint->disconnectFromHost();
}

bool JsonRpcClient::isConnected() const
{
    return m_endpoint->isConnected();
}

QHostAddress JsonRpcClient::clientAddress() const
{
    return m_endpoint->localAddress();
}

int JsonRpcClient::clientPort() const
{
    return m_endpoint->localPort();
}

QHostAddress JsonRpcClient::serverAddress() const
{
    return m_endpoint->peerAddress();
}

int JsonRpcClient::serverPort() const
{
    return m_endpoint->peerPort();
}

void JsonRpcClient::jsonResponseReceived(const QJsonObject& response)
{
    RequestId id = QUuid(response.value("id").toString());
    if (response.value("error").isObject()) {
        int code;
        QString msg;
        QVariant data;
        getJsonErrorInfo(response, code, msg, data);
        logError(QString("(%1) - %2").arg(code).arg(msg));

        {
            auto it = m_outstanding_requests.find(id);
            if (it == m_outstanding_requests.end()) {
                logError(QString("got error response for non-existing "
                                 "request: %1").arg(QUuid(id).toString()));
                return;
            }
            emit it.value()->error(code, msg, data);
            m_outstanding_requests.erase(it);
            --m_outstanding_request_count;
        }

        return;
    }

    if (response["result"].isUndefined()) {
        logError("result is undefined");
        return;
    }

    QVariant result = response.value("result").toVariant();

    auto it = m_outstanding_requests.find(id);
    if (it == m_outstanding_requests.end()) {
        logError(QString("got response to non-existing request: %1").arg(QUuid(id).toString()));
        return;
    }

    emit it.value()->result(result);
    m_outstanding_requests.erase(it);
    --m_outstanding_request_count;
}

void JsonRpcClient::getJsonErrorInfo(const QJsonObject& response,
                                     int& code,
                                     QString& message,
                                     QVariant& data)
{
    QJsonObject error = response["error"].toObject();
    code = error["code"].toInt();
    message = error["message"].toString("unknown error");
    data = error.value("data").toVariant();
}

QString JsonRpcClient::formatLogMessage(const QString& method,
                                        const QVariantList& args,
                                        bool async,
                                        const RequestId& request_id)
{
    auto msg = QString("Calling (%1) RPC method: '%2' ")
        .arg(async ? "async" : "sync").arg(method);

    if (args.empty()) {
        msg += "without arguments";
    } else {
        msg += QString("with argument%1: %2")
            .arg(args.size() == 1 ? "" : "s")
            .arg(variantListToStringList(args).join(", "));
    }
    msg += QString(" (request ID: %1)").arg(request_id.toString());
    return msg;
}

void JsonRpcClient::logError(const QString& msg)
{
    m_logger->logError("JSON RPC client error: " + msg);
}

}
