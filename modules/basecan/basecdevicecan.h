#ifndef BASECDEVICECAN_H
#define BASECDEVICECAN_H

#include "cdevice.h"

class QCanBusFrame;
/*!
 * \brief Абстрактный класс устройств работающих по Can
 */
class BaseCdeviceCan : public CDevice
{
public:
    /*!
     * \brief BaseCdeviceCan - конструктор девайса Can
     * \param addr - адрес, записанный в виде ("ip:port")
     */
    explicit BaseCdeviceCan(const std::string& addr);

    /*!
     * \brief read - чтение в dataframe
     * \param dataFrame - дата формата QCanBus
     * \return Cdevice::SUCCESS или CDevice::ERROR
     */
    virtual int read(QByteArray& dataFrame) = 0;

    /*!
     * \brief write - запись dataframe
     * \param dataFrame - дата формата QCanBus
     * \return Cdevice::SUCCESS или CDevice::ERROR
     */
    virtual int write(const QCanBusFrame& dataFrame) = 0;

    /*!
     * \brief connectToServer - подключение к серверу
     * \return Cdevice::SUCCESS или CDevice::ERROR
     */
    virtual int connectToServer() = 0;

    /*!
     * \brief Проверка соединения
     * \return True - если соединение установлено
     */
    virtual bool isConnected() = 0;

protected:
    /*!
    * \brief Пользовательская реализация метода установки соединения
   */
    virtual int onInit() = 0;

    /*!
     * \brief Пользовательская реализация метода закрытия соединения
     */
    virtual int onClose() = 0;



private:
    Q_DISABLE_COPY_MOVE(BaseCdeviceCan);

};

#endif // BASECDEVICECAN_H
