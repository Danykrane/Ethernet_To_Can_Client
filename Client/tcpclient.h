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
    bool connectToServer(const QString& hostName, quint16 port);
    void disconnectFromServer();
    void sendData(const QByteArray& data);

signals:
    void connectedToServer();
    void disconnectedFromServer();
    void dataReceived(const QByteArray& data);

public slots:
    void readData();

private:
    QTcpSocket* socket;
};

#endif // TCPCLIENT_H