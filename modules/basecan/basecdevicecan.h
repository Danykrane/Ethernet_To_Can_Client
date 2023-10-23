#ifndef BASECDEVICECAN_H
#define BASECDEVICECAN_H

#include "cdevice.h"

class QCanBusFrame;

class BaseCdeviceCanPrivate;
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

protected:
    /*!
    * \brief Пользовательская реализация метода установки соединения
   */
    virtual int onInit() override;

    /*!
     * \brief Пользовательская реализация метода закрытия соединения
     */
    virtual int onClose() override;

    /*!
     * \brief True - если соединение установлено
     * \return
     */
    virtual bool isConnected() = 0;

    /*!
     * \brief readData - чтение dataframe
     * \param dataFrame - дата формата QCanBus
     * \return
     */
    virtual int read(const QCanBusFrame& dataFrame) = 0;

    /*!
     * \brief writeData - запись dataframe
     * \param dataFrame - дата формата QCanBus
     * \return
     */
    virtual int write(const QCanBusFrame& dataFrame) = 0;

private:
    Q_DISABLE_COPY_MOVE(BaseCdeviceCan);

};

#endif // BASECDEVICECAN_H
