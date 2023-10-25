#include "tcpclient.h"

#include <QTime>

TcpClient::TcpClient(QObject* parent)
    : QObject(parent)
{
    socket = new QTcpSocket(this);

    connect(
        socket, &QTcpSocket::connected, this, &TcpClient::connectedToServer);

    connect(socket,
            &QTcpSocket::disconnected,
            this,
            &TcpClient::disconnectedFromServer);

    connect(socket, &QTcpSocket::readyRead, this, &TcpClient::read);
}


bool TcpClient::connectToServer(const QString& hostName, uint16_t port, uint16_t waitMsec)
{
    socket->connectToHost(hostName, port);
    return socket->waitForConnected(waitMsec);
}

void TcpClient::disconnectFromServer()
{
    socket->disconnectFromHost();
}

bool TcpClient::sendData(const QByteArray& data)
{
    socket->write(data);
    socket->flush();
    return socket->waitForBytesWritten();

}

void TcpClient::read()
{
    QByteArray data = socket->readAll();
    emit       dataReceived(data);
}
