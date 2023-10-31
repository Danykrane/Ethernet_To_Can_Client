#include "cdeviceusrcanet200.h"
#include "client/tcpclient.h"
#include "TupleAddressParser.h"

#include <QRegularExpression>
#include <QCanBusFrame>
#include <QVector>
#include <QQueue>
#include <QThread>
#include <QEventLoop>
#include <QEvent>

// для взаимодействия со слотами в другом event loop
#include <QMetaObject>
#include <QAbstractEventDispatcher>

/* ------------------------------- parseFunctions -------------------------- */
const uint8_t canDataSize = 13;
const uint8_t canPayloadSize = 8;
const uint8_t canIdByteSize = 4;
const uint8_t startPayloadIndex = 5;
const uint8_t startCanIdIndex = 1;

/*!
 * \brief parseCanFrameToByteArray - convert CanBusFrame в QByteArray
 * \param canFrame - кадр формата Can
 * \return данные в QByteArray
 */
QByteArray parseCanFrameToByteArray(const QCanBusFrame& canFrame){
    QByteArray resData;
    resData.resize(canDataSize);

    uint16_t payloadSize = canFrame.payload().size();

    // frame info 1 byte
    // маска FF
    resData[0] = canFrame.payload().size() & 0xFF;

    //frame id {2 byte}, но записываем в 4 байта
    quint32 tempFrameId = canFrame.frameId();

    // сдвиг вправо + маска 0xFF
    for(int i = 0; i < canIdByteSize; ++i){
        resData[canIdByteSize - i] = (tempFrameId >> (8 * i)) & 0xFF;
    }

    // payload data
    for(int i = 0; i < payloadSize; ++i){
        // со смещением на 5 тк payload data начинается с 5 байта
        resData[i + startPayloadIndex] = *(canFrame.payload().begin() + i);
    }

    // empty elements
    if(payloadSize < canPayloadSize){
        for(int i = payloadSize; i < canPayloadSize; ++i){
            resData[i + startPayloadIndex] = 0;
        }
    }
    return resData;
}

/*!
 * \brief parseByteArrayToCanFrames - convert QbyteArray в очередь кдров Can
 * \param curArray - текущий QByteArray
 * \param isOk - статус выполнения ф-ии
 * \return данные в формате QQueue<QCanBusFrame
 */
QQueue<QCanBusFrame> parseByteArrayToCanFrames(QByteArray &curArray, bool &isOk){
    QQueue<QCanBusFrame> resFrames;
    isOk = true;
    if(curArray.size() == 0){
        isOk = false;
        return {};
    }

    uint16_t colFullFrames = curArray.size() / canDataSize;
    uint16_t ostArrData = curArray.size()  - colFullFrames * canDataSize;

    //заполним очередь
    for(int i = 0; i < colFullFrames; i+= canDataSize){
        QCanBusFrame temp;

        // frame id
        QByteArray frameId;
        // считаем 4 байта id
        for(int ind = 0; ind < canIdByteSize; ++ind){
            frameId.append(curArray[i + ind + startCanIdIndex]);
        }
        // записываем в переменную со сдвигом
        quint32 curFrameId = 0;
        for(int ind = 0; ind < canIdByteSize;++ind){
            curFrameId += static_cast<uint8_t>(frameId[ind] & 0xFF) << (8 * (canIdByteSize - i - 1));
        }

        temp.setFrameId(curFrameId);


        // payload
        uint8_t currPayloadSize = static_cast<uint8_t>(curArray[i] & 0x0F);
        QByteArray payload;
        for(int ind = 0; ind < currPayloadSize; ++ind){
            payload.append(curArray[i + ind + startPayloadIndex]);
        }
        temp.setPayload(payload);

        // добавляем в очередь
        resFrames << temp;
    }

    // в случае обрывистого сообщения
    if(ostArrData > 0){
        curArray.remove(0,colFullFrames * canDataSize);
    }else{
        curArray.clear();
    }

    return resFrames;
}


/* ---------------------------- CdeviceUsrCanet200Private ------------------ */
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

    // текущие данные в QByteArray
    QByteArray currentSocketData;

    // текущие данные по фреймам
    QQueue<QCanBusFrame> currentAllFrames;

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
    bool disconnectDevice();

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

bool CdeviceUsrCanet200Private::disconnectDevice()
{
    return client->disconnectFromServer(waitMsec);
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
        // для теста сформируем команды в формате одного QByteArray
        currentSocketData += data;
        qDebug() <<currentSocketData<<"=========";
        bool isOk = false;
        currentAllFrames += parseByteArrayToCanFrames(currentSocketData, isOk);
//        emit q->sendedFromSocketData();
        }, Qt::QueuedConnection);

    /*
     *
     *  Первая партия данных
     *
     * "\x85\x12""4Vx\x01\x02\x03\x04\x05\x00\x00\x00\
     *   b\x00\x00\x00\x0E\x00\x00\x00\x00\x00\x00\x00\x1B\
     *   x85\x12""4Vx\x01\x02\x03\x04\x05\x00\x00\x00\
     *   b\x00\x00\x00\x0E\x00\x00\x00\x00\x00\x00\x00\x1B\
     *   x85\x12""4Vx\x01\x02\x03\x04\x05\x00\x00\x00\
     *   x85\x12""4Vx\x01\x02\x03\x04\x05\x00\x00\x00\
     *   x85\x12""4Vx\x01\x02\x03\x04\x05\x00\x00\x00\
     *   x85\x12""4Vx\x01\
     *
     *  Вторая партия данных
     *                      x02\x03\x04\x05\x00\x00\x00\
     *   b\x00\x00\x00\x0E\x00\x00\x00\x00\x00\x00\x00\x1B\
     *   x85\x12""4Vx\x01\x02\x03\x04\x05\x00\x00\x00\
     *   b\x00\x00\x00\x0E\x00\x00\x00\x00\x00\x00\x00\x1B\
     *   b\x00\x00\x00\x0E\x00\x00\x00\x00\x00\x00\x00\x1B\
     *
     */
}


/* ------------------------------ CdeviceUsrCanet200 ----------------------- */
CdeviceUsrCanet200::CdeviceUsrCanet200(const std::string &addr):
    BaseCdeviceCan(addr),
    d_ptr(new CdeviceUsrCanet200Private(this))
{
    Q_D(CdeviceUsrCanet200);

    // парсинг адреса на [имя хоста]:[порт]
    std::string ipStr;
    int port = 0;
    bool parseRes = false;
    std::tie(ipStr, port, parseRes) = tap::parse<std::string, int>(addr, ':');

    if(parseRes){
        d->hostName = QString::fromStdString(ipStr);
        d->port = port;
    }

}

int CdeviceUsrCanet200::readFrame(QCanBusFrame &dataFrame)
{
    Q_D(CdeviceUsrCanet200);
    if(d->currentAllFrames.size() == 0){
        return CDevice::error;
    }
    dataFrame = d->currentAllFrames.dequeue();
    return CDevice::SUCCESS;
}

int CdeviceUsrCanet200::readAllFrames(QQueue<QCanBusFrame> &dataFrames)
{
    Q_D(CdeviceUsrCanet200);
    if(d->currentAllFrames.size() == 0){
        return CDevice::error;
    }
    dataFrames = d->currentAllFrames;
    d->currentAllFrames.clear();
    return CDevice::SUCCESS;
}

int CdeviceUsrCanet200::write(const QCanBusFrame &dataFrame)
{
    //TODO[new]:: исправить, чтобы запускалось в отдельном потоке
    Q_D(CdeviceUsrCanet200);
    bool isOk = false;

    auto writeLabda = [=]() -> bool{
        if(d->client){
            return d->client->sendData(parseCanFrameToByteArray(dataFrame));
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
            // удаление объекта при disconnected сигнала QTcpSocket
            QObject::connect(d->client,
                             &TcpClient::disconnectedFromServer,
                d->client,[=](){
                    d->client->deleteLater();
                    d->client = nullptr;
                },Qt::QueuedConnection);


        }

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
    bool isOk = false;
    auto close = [=]() -> bool{
        d->connectionStatus = false;
        return d->disconnectDevice();
    };

    QMetaObject::invokeMethod(
        QAbstractEventDispatcher::instance(d->m_thread.get()),
        close, Qt::BlockingQueuedConnection, &isOk);

    if(isOk){
       return CDevice::SUCCESS;
    }
    return CDevice::error;
}

int CdeviceUsrCanet200::connectToServer()
{
    Q_D(CdeviceUsrCanet200);
    bool isOk = false;
    auto connect = [=]() -> bool{
        if(d->client == nullptr){
            d->client = new TcpClient;
            d->createConnections();
            QObject::connect(d->client,
                &TcpClient::disconnectedFromServer,
                d->client,[=](){
                    d->client->deleteLater();
                    d->client = nullptr;
                },Qt::QueuedConnection);
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


