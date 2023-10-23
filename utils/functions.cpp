#include <QByteArray>
#include <QCanBusFrame>

const uint16_t canDataSize = 13;

QByteArray parseCanFrame(const QCanBusFrame& canFrame){
    QByteArray resData;
    resData.resize(canDataSize);
    uint16_t payloadSize = canFrame.payload().size();
//    resData[0] =



    return resData;
}
