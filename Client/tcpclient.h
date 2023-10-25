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
    void disconnectFromServer();

public slots:
    void read();
    bool sendData(const QByteArray& data);

signals:
    void connectedToServer();
    void disconnectedFromServer();
    void dataReceived(const QByteArray& data);

private:
    QTcpSocket* socket;
};

#endif // TCPCLIENT_H
