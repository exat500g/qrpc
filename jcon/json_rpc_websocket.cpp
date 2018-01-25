#include "json_rpc_websocket.h"
#include "jcon_assert.h"

#include <QTime>
#include <QWebSocket>
#include <QCoreApplication>

namespace jcon {

JsonRpcWebSocket::JsonRpcWebSocket()
    : m_socket(new QWebSocket)
{
    setupSocket();
}

JsonRpcWebSocket::JsonRpcWebSocket(QWebSocket* socket)
    : m_socket(socket)
{
    setupSocket();
}

JsonRpcWebSocket::~JsonRpcWebSocket()
{
    m_socket->disconnect(this);
    m_socket->deleteLater();
}

void JsonRpcWebSocket::setupSocket()
{
    connect(m_socket, &QWebSocket::connected, this, [this]() {
        emit socketConnected(m_socket);
    });

    connect(m_socket, &QWebSocket::disconnected, this, [this]() {
        emit socketDisconnected(m_socket);
    });

    connect(m_socket, &QWebSocket::binaryMessageReceived,
            this, &JsonRpcWebSocket::onMessageReceived);

    void (QWebSocket::*errorPtr)(QAbstractSocket::SocketError) =
        &QWebSocket::error;
    connect(m_socket, errorPtr, this,
            [this](QAbstractSocket::SocketError error) {
                emit socketError(m_socket, error);
            });
}

void JsonRpcWebSocket::connectToHost(const QString& host, int port)
{
    QUrl url;
    url.setScheme("ws");
    url.setHost(host);
    url.setPort(port);
    m_socket->open(url);
}

void JsonRpcWebSocket::connectToUrl(const QUrl& url)
{
    m_socket->open(url);
}

bool JsonRpcWebSocket::waitForConnected(int msecs)
{
    QTime timer(0, 0, 0, msecs);
    timer.start();
    while (this->isConnected()==false && timer.elapsed() < msecs) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    return isConnected();
}

void JsonRpcWebSocket::disconnectFromHost()
{
    m_socket->close();
}

bool JsonRpcWebSocket::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

size_t JsonRpcWebSocket::send(const QByteArray& data)
{
    return m_socket->sendBinaryMessage(data);
}

QString JsonRpcWebSocket::errorString() const
{
    return m_socket->errorString();
}

QHostAddress JsonRpcWebSocket::localAddress() const
{
    return m_socket->localAddress();
}

int JsonRpcWebSocket::localPort() const
{
    return m_socket->localPort();
}

QHostAddress JsonRpcWebSocket::peerAddress() const
{
    return m_socket->peerAddress();
}

int JsonRpcWebSocket::peerPort() const
{
    return m_socket->peerPort();
}

void JsonRpcWebSocket::onMessageReceived(const QByteArray &message){
    emit messageReceived(message,m_socket);
}

}
