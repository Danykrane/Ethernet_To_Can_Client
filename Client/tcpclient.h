#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QtCore>
#include <QtNetwork>

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject* parent = nullptr);

    QByteArray socketData();

public slots:
    bool connectToServer(const QString& hostName, uint16_t port, uint16_t waitMsec);
    void disconnectFromServer();
    void sendData(const QByteArray& data);

signals:
    void connectedToServer();
    void disconnectedFromServer();
    void dataReceived(const QByteArray& data);

public slots:
    void read();

private:
    QTcpSocket* socket;
};

#endif // TCPCLIENT_H
