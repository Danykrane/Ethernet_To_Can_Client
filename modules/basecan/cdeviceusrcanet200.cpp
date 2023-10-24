#include "cdeviceusrcanet200.h"
#include "client/tcpclient.h"
#include "utils/functions.cpp"

#include <QRegularExpression>
#include <QVector>


/* ---------------------------- CdeviceUsrCanet200Private ---------------- */
class CdeviceUsrCanet200Private
{
public:
    CdeviceUsrCanet200Private(CdeviceUsrCanet200 *parent);
public:
    // tcp клиент
    TcpClient *client;
    // заменить на QTCp socket

    // имя хоста
    QString hostName;

    // номер порта
    uint16_t port;

    // статус соединения
    bool connectionStatus = false;

    // время ожидания соединения
    uint16_t waitMsec = 500;

    // текущие данные
    QByteArray currentSocketData;
public:
    /*!
     * \brief connectToHost - создать соединение из записанных данных
     * \param waitMsec - ожидание соединения в мСек
     * \return true - соединение установлено, false - проблема с соединением
     */
    bool connectToHost(uint16_t waitMsec);

    /*!
     * \brief disconnectDevice - отключение устройства
     */
    void disconnectDevice();

    /*!
     * \brief parseAdress - Парсинг адреса на ip и порт
     * \param adress - строка Ip:port
     */
    void parseAdress(const std::string& adress);

    /*!
     * \brief Создание коннектов
     */
    void createConnections();

public:
    Q_DECLARE_PUBLIC(CdeviceUsrCanet200);
    CdeviceUsrCanet200 *q_ptr;
};

CdeviceUsrCanet200Private::CdeviceUsrCanet200Private(CdeviceUsrCanet200 *parent):
    client(new TcpClient), q_ptr(parent)
{
    createConnections();
}

bool CdeviceUsrCanet200Private::connectToHost(uint16_t waitMsec)
{
    return client->connectToServer(hostName, port, waitMsec);
}

void CdeviceUsrCanet200Private::disconnectDevice()
{
    client->disconnectFromServer();
}

void CdeviceUsrCanet200Private::parseAdress(const std::string &adress)
{
    QRegularExpression ipPattern("([0-9]{1,3}[\\.]){3}[0-9]{1,3}");
    QRegularExpression portPattern(":(\\d+)");

    QRegularExpressionMatch ipMatch = ipPattern.match(QString::fromStdString(adress));
    QRegularExpressionMatch portMatch = portPattern.match(QString::fromStdString(adress));

    if(ipMatch.hasMatch()){
        hostName = ipMatch.captured();
    }

    if(portMatch.hasMatch()){
        port = portMatch.captured(1).toInt();
    }

}

void CdeviceUsrCanet200Private::createConnections()
{
//    Q_Q(CdeviceUsrCanet200);
    QAbstractEventDispatcher::connect(client, &TcpClient::connectedToServer, [] () {
        qDebug() << "Connected to server";
    });

    QAbstractEventDispatcher::connect(client, &TcpClient::disconnectedFromServer, [] () {
        qDebug() << "Disconnected from server";
    });

    QAbstractEventDispatcher::connect(client, &TcpClient::dataReceived,[=](const QByteArray &data){
        qDebug() <<data<<"=========";
        currentSocketData = data;
//        emit q->sendedFromSocketData();
    });
}


/* ------------------------------ CdeviceUsrCanet200 ----------------------- */
CdeviceUsrCanet200::CdeviceUsrCanet200(const std::string &addr):
    BaseCdeviceCan(addr),
    d_ptr(new CdeviceUsrCanet200Private(this))
{
    Q_D(CdeviceUsrCanet200);
    d->parseAdress(addr);
}

int CdeviceUsrCanet200::read(QByteArray &dataFrame)
{
    Q_D(CdeviceUsrCanet200);
    dataFrame = d->currentSocketData;
    return CDevice::SUCCESS;
}

int CdeviceUsrCanet200::write(const QCanBusFrame &dataFrame)
{
    //TODO[new]:: исправить, чтобы запускалось в отдельном потоке
    Q_D(CdeviceUsrCanet200);
    if (d->client->sendData(parseCanFrame(dataFrame))){
        return CDevice::SUCCESS;

    }
    return CDevice::error;

}

int CdeviceUsrCanet200::onInit()
{
    Q_D(CdeviceUsrCanet200);
    d->connectionStatus = d->connectToHost(d->waitMsec);
    if(d->connectionStatus){
        return CDevice::SUCCESS;
    }
    return CDevice::error;
}

int CdeviceUsrCanet200::onClose()
{
    Q_D(CdeviceUsrCanet200);
    d->connectionStatus = false;
    d->disconnectDevice();
    return CDevice::SUCCESS;
}

bool CdeviceUsrCanet200::isConnected()
{
    Q_D(CdeviceUsrCanet200);
    return d->connectionStatus;
}

void CdeviceUsrCanet200::setHostName(const QString &hostName)
{
    Q_D(CdeviceUsrCanet200);
    d->hostName = hostName;
}

void CdeviceUsrCanet200::setPort(uint16_t port)
{
    Q_D(CdeviceUsrCanet200);
    d->port = port;

}

const QString CdeviceUsrCanet200::hostName()
{
    Q_D(CdeviceUsrCanet200);
    return d->hostName;
}

uint16_t CdeviceUsrCanet200::port()
{
    Q_D(CdeviceUsrCanet200);
    return d->port;
}

void CdeviceUsrCanet200::setWaitConnectTime(uint16_t waitMsec)
{
    Q_D(CdeviceUsrCanet200);
    d->waitMsec = waitMsec;
}

const uint16_t CdeviceUsrCanet200::waitMsec()
{
    Q_D(CdeviceUsrCanet200);
    return d->waitMsec;
}


