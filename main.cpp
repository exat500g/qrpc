#include "example_service.h"

#include <jcon/json_rpc_websocket_client.h>
#include <jcon/json_rpc_websocket_server.h>

#include <QCoreApplication>
#include <QPushButton>
#include <QThread>
#include <QUrl>

#include <ctime>
#include <iostream>
#include <memory>

void startServer(QObject* parent)
{
    jcon::JsonRpcServer* rpc_server;
    {
        qDebug() << "Creating WebSocket server";
        rpc_server = new jcon::JsonRpcWebSocketServer(parent);
    }
    auto service = new ExampleService;
    rpc_server->registerServices({ service });
    rpc_server->listen(6002);
}

jcon::JsonRpcClient* startClient(QObject* parent)
{
    jcon::JsonRpcClient* rpc_client;
    {
        rpc_client = new jcon::JsonRpcWebSocketClient(parent);
        // This is just to illustrate the fact that connectToServer also accepts
        // a QUrl argument.
        rpc_client->connectToServer(QUrl("ws://127.0.0.1:6002"));
    }
    return rpc_client;
}

void invokeMethodAsync(jcon::JsonRpcClient* rpc_client)
{
    qsrand(std::time(nullptr));

    auto req = rpc_client->callAsync("getRandomInt", 10);

    req->connect(req.get(), &jcon::JsonRpcRequest::result,
                 [](const QVariant& result) {
                     qDebug() << "result of asynchronous RPC call:" << result;
                 });

    req->connect(req.get(), &jcon::JsonRpcRequest::error,
                 [](int code, const QString& message, const QVariant& ) {
                     qDebug() << "RPC error:" << message
                              << " (" << code << ")";
                 });
}

void invokeMethodSync(jcon::JsonRpcClient* rpc_client)
{
    qsrand(std::time(nullptr));

    auto result = rpc_client->call("getRandomInt", 100);

    if (result->isSuccess()) {
        qDebug() << "result of synchronous RPC call:" << result->result();
    } else {
        qDebug() << "RPC error:" << result->toString();
    }
    {
        qDebug()<<"test printU64";
        auto result = rpc_client->call("printU64", 4611686018427387905);

        if (result->isSuccess()) {
            qDebug() << "result of synchronous RPC call:" << result->result();
        } else {
            qDebug() << "RPC error:" << result->toString();
        }
    }
}

void invokeStringMethodAsync(jcon::JsonRpcClient* rpc_client)
{
    auto req = rpc_client->callAsync("printMessage", "hello, world");

    req->connect(req.get(), &jcon::JsonRpcRequest::result,
                 [](const QVariant& result) {
                     qDebug() << "result of asynchronous RPC call:" << result;
                 });

    req->connect(req.get(), &jcon::JsonRpcRequest::error,
                 [](int code, const QString& message, const QVariant& ) {
                     qDebug() << "RPC error:" << message
                              << " (" << code << ")";
                 });
}

void invokeStringMethodSync(jcon::JsonRpcClient* rpc_client)
{
    qsrand(std::time(nullptr));

    auto result = rpc_client->call("printMessage", "hello, world");

    if (result->isSuccess()) {
        qDebug() << "result of synchronous RPC call:" << result->result();
    } else {
        qDebug() << "RPC error:" << result->toString();
    }
}

#include "msgpack/msgpack.h"

QByteArray pack_quuid(const QVariant &variant)
{
    QUuid uuid = variant.toUuid();
    return uuid.toRfc4122();
}

QVariant unpack_quuid(const QByteArray &data)
{
    return QUuid::fromRfc4122(data);
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    startServer(&app);
    auto rpc_client = startClient(&app);

    invokeMethodAsync(rpc_client);
    invokeMethodSync(rpc_client);
    invokeStringMethodSync(rpc_client);
    invokeStringMethodAsync(rpc_client);



    if (rpc_client->outstandingRequestCount() > 0) {
        qDebug().noquote() << QString("Waiting for %1 outstanding requests")
            .arg(rpc_client->outstandingRequestCount());

        while (rpc_client->outstandingRequestCount() > 0) {
            qDebug() << "Calling QCoreApplication::processEvents()";
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    } else {
        qDebug() << "No outstanding requests, quitting";
    }

    {
        MsgPack::registerPacker(QMetaType::QUuid,QMetaType::QUuid,pack_quuid);
        MsgPack::registerUnpacker(QMetaType::QUuid,unpack_quuid);
        QVariantMap map;
        map["id"]=QUuid::createUuid();
        //map["value"]="asdfsdf";
        //map["version"]=4611686018427387905;
        qDebug()<<"map="<<map;
        QByteArray bin=MsgPack::pack(map);
        {
            QVariant res=MsgPack::unpack(bin);
            qDebug()<<"res="<<res<<"bin.length="<<bin.length();
        }
    }

    return 0;
}
