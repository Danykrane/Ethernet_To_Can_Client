#include <QByteArray>
#include <QCanBusFrame>

const uint8_t canDataSize = 13;
const uint8_t canPayloadSize = 8;
const uint8_t canIdByteSize = 4;
const uint8_t startPayloadPosion = 5;

/*!
 * \brief parseCanFrame - парсинг CanBusFrame в QByteArray
 * \param canFrame - кадр формата Can
 * \return данные в QByteArray
 */
QByteArray parseCanFrame(const QCanBusFrame& canFrame){
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
        resData[i + startPayloadPosion] = *(canFrame.payload().begin() + i);
    }

    // empty elements
    if(payloadSize < canPayloadSize){
        for(int i = payloadSize; i < canPayloadSize; ++i){
            resData[i + startPayloadPosion] = 0;
        }
    }
    return resData;
}
