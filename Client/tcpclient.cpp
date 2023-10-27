#include "tcpclient.h"

#include <QTime>

TcpClient::TcpClient(QObject* parent)
    : QObject(parent)
{
    m_socket = new QTcpSocket(this);

    connect(
        m_socket, &QTcpSocket::connected, this, &TcpClient::connectedToServer);

    connect(m_socket,
            &QTcpSocket::disconnected,
            this,
            &TcpClient::disconnectedFromServer);

    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::read);
}


bool TcpClient::connectToServer(const QString& hostName, uint16_t port, uint16_t waitMsec)
{
    m_socket->connectToHost(hostName, port);
    return m_socket->waitForConnected(waitMsec);
}

bool TcpClient::disconnectFromServer(uint16_t waitMsec)
{
    m_socket->disconnectFromHost();
    bool res = m_socket->state() == QAbstractSocket::UnconnectedState
                   || m_socket->waitForDisconnected(waitMsec);

    return res;
}

bool TcpClient::sendData(const QByteArray& data)
{
    m_socket->write(data);
    m_socket->flush();
    return m_socket->waitForBytesWritten();

}


void TcpClient::read()
{
    QByteArray data = m_socket->readAll();
    emit       dataReceived(data);
}
