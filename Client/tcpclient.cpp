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

    connect(socket, &QTcpSocket::readyRead, this, &TcpClient::readData);
}

QByteArray TcpClient::socketData()
{
    return (QTime::currentTime().toString() + " ").toUtf8() + socket->readAll();
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

void TcpClient::sendData(const QByteArray& data)
{
    socket->write(data);
    bool writeFlag = socket->waitForBytesWritten();
    if (writeFlag)
    {
        qDebug() << "socket written: " << data;
    }
    else
    {
        qDebug() << "probleb with write";
    }
}

void TcpClient::readData()
{
    QByteArray data = socket->readAll();
    emit       dataReceived(data);
}
