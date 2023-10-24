#include "cdeviceusrcanet200.h"
#include "client/tcpclient.h"

#include <QRegularExpression>


/* ---------------------------- CdeviceUsrCanet200Private ---------------- */
class CdeviceUsrCanet200Private
{
public:
    CdeviceUsrCanet200Private();
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

    uint16_t waitMsec = 500;

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

    void parseAdress(const std::string& adress);
};

CdeviceUsrCanet200Private::CdeviceUsrCanet200Private():
    client(new TcpClient)
{

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
        port = portMatch.captured(0).toInt();
    }

}


///* ------------------------------ CdeviceUsrCanet200 --------------------- */
CdeviceUsrCanet200::CdeviceUsrCanet200(const std::string &addr):
    BaseCdeviceCan(addr),
    d_ptr(new CdeviceUsrCanet200Private)
{
    Q_D(CdeviceUsrCanet200);
    d->parseAdress(addr);
}

int CdeviceUsrCanet200::read(QCanBusFrame &dataFrame)
{
    return CDevice::SUCCESS;

}

int CdeviceUsrCanet200::write(const QCanBusFrame &dataFrame)
{
    //TODO[new]:: исправить, чтобы запускалось в отдельном потоке
    Q_D(CdeviceUsrCanet200);
//    QByteArray currData = parseCanFrame(dataFrame);
//    d->client->sendData(currData);

    return CDevice::SUCCESS;
}

int CdeviceUsrCanet200::onInit()
{
    //TODO[new]:: исправить, чтобы запускалось в отдельном потоке
    Q_D(CdeviceUsrCanet200);
    d->connectionStatus = d->connectToHost(d->waitMsec);
    if(d->connectionStatus){
        return CDevice::SUCCESS;
    }
    return CDevice::error;
}

int CdeviceUsrCanet200::onClose()
{
    //TODO[new]:: исправить, чтобы закрывалось из другого потока
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


