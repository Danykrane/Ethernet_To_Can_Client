#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QtCore>
#include <QtNetwork>

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject* parent = nullptr);

public:
    bool connectToServer(const QString& hostName, uint16_t port, uint16_t waitMsec);
    bool disconnectFromServer(uint16_t waitMsec);

public slots:
    void read();
    bool sendData(const QByteArray& data);

signals:
    void connectedToServer();
    void disconnectedFromServer();
    void dataReceived(const QByteArray& data);

private:
    QTcpSocket* m_socket;
};

#endif // TCPCLIENT_H
