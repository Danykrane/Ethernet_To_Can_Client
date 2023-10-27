#include "cdeviceusrcanet200.h"
#include "client/tcpclient.h"
#include "utils/functions.cpp"

#include <QRegularExpression>
#include <QVector>
#include <QThread>
#include <QEventLoop>
#include <QEvent>

// для взаимодействия со слотами в другом event loop
#include <QMetaObject>
#include <QAbstractEventDispatcher>


/* ---------------------------- CdeviceUsrCanet200Private ---------------- */
class CdeviceUsrCanet200Private
{
public:
    explicit CdeviceUsrCanet200Private(CdeviceUsrCanet200 *parent);
    ~CdeviceUsrCanet200Private();
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

    // поток для сокета
    std::unique_ptr<QThread> m_thread;
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
    client(nullptr),
    m_thread(new QThread),
    q_ptr(parent)
{
    QEventLoop loop;

    QObject::connect(m_thread.get(),&QThread::started,&loop,&QEventLoop::quit,Qt::QueuedConnection);
    // запускаем поток
    m_thread->start();
    // входим в eventLoop
    loop.exec();

}

CdeviceUsrCanet200Private::~CdeviceUsrCanet200Private()
{
    m_thread->quit();
    m_thread->wait();
    m_thread->deleteLater();
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
    QObject::connect(client, &TcpClient::connectedToServer,client, [] () {
        qDebug() << "Connected to server";
        },Qt::QueuedConnection);

    QObject::connect(client, &TcpClient::disconnectedFromServer,client, [] () {
        qDebug() << "Disconnected from server";
        }, Qt::QueuedConnection);

    QObject::connect(client, &TcpClient::dataReceived,client,[=](const QByteArray &data){
        qDebug() <<data<<"=========";
        currentSocketData = data;
//        emit q->sendedFromSocketData();
        }, Qt::QueuedConnection);
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
    bool isOk = false;

    auto writeLabda = [=]() -> bool{
        if(d->client){
            return d->client->sendData(parseCanFrame(dataFrame));
        }
        return false;
    };

    QMetaObject::invokeMethod(
        QAbstractEventDispatcher::instance(d->m_thread.get()),
        writeLabda,Qt::BlockingQueuedConnection, &isOk);

    if (isOk){
        return CDevice::SUCCESS;
    }
    return CDevice::error;

}

int CdeviceUsrCanet200::onInit()
{
    Q_D(CdeviceUsrCanet200);
    bool isOk = false;
    auto initTcp = [=]() -> bool{
        if (!d->client) {
            d->client = new TcpClient();
            d->createConnections();
//            // NOTE:: удаление объекта при disconnected сигнала QTcpSocket
//            QObject::connect(d->client,
//                             &TcpClient::disconnectedFromServer,
//                             d->client,
//                             &TcpClient::deleteLater,
//                             Qt::QueuedConnection);
        }
        // перемещаем объект в поток
        //FIXME: Понять, почему бессмысленно
//        d->client->moveToThread(d->m_thread.get());
        d->connectionStatus = d->connectToHost(d->waitMsec);
        return d->connectionStatus;
    };

    QMetaObject::invokeMethod(
        QAbstractEventDispatcher::instance(d->m_thread.get()),
        initTcp, Qt::BlockingQueuedConnection, &isOk);


    if(isOk){
       return CDevice::SUCCESS;
    }
    return CDevice::error;
}

int CdeviceUsrCanet200::onClose()
{
    Q_D(CdeviceUsrCanet200);

    auto close = [=](){
        d->connectionStatus = false;
        d->disconnectDevice();
    };

    QMetaObject::invokeMethod(
        QAbstractEventDispatcher::instance(d->m_thread.get()),
        close, Qt::BlockingQueuedConnection);

    return CDevice::SUCCESS;
}

int CdeviceUsrCanet200::connectToServer()
{
    Q_D(CdeviceUsrCanet200);
    bool isOk = false;
    auto connect = [=]() -> bool{
        if(!d->client){
            return false;
        }

        d->connectionStatus = d->connectToHost(d->waitMsec);
        return d->connectionStatus;

    };

    QMetaObject::invokeMethod(
        QAbstractEventDispatcher::instance(d->m_thread.get()),
        connect, Qt::BlockingQueuedConnection, &isOk);

    if(isOk){
       return CDevice::SUCCESS;
    }
    return CDevice::error;
}

bool CdeviceUsrCanet200::isConnected()
{
    Q_D(CdeviceUsrCanet200);
    bool res = false;

    auto connected = [=]() -> bool{
        return d->connectionStatus;
    };

    QMetaObject::invokeMethod(
        QAbstractEventDispatcher::instance(d->m_thread.get()),
        connected, Qt::BlockingQueuedConnection, &res);

    return res;
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

uint16_t CdeviceUsrCanet200::waitMsec()
{
    Q_D(CdeviceUsrCanet200);
    return d->waitMsec;
}


