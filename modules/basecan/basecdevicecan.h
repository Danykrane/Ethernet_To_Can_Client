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

protected:
    /*!
    * \brief Пользовательская реализация метода установки соединения
   */
    virtual int onInit() = 0;

    /*!
     * \brief Пользовательская реализация метода закрытия соединения
     */
    virtual int onClose() = 0;

    /*!
     * \brief True - если соединение установлено
     * \return
     */
    virtual bool isConnected() = 0;

    /*!
     * \brief read - чтение dataframe
     * \param dataFrame - дата формата QCanBus
     * \return
     */
    virtual int read(QCanBusFrame& dataFrame) = 0;

    /*!
     * \brief write - запись dataframe
     * \param dataFrame - дата формата QCanBus
     * \return
     */
    virtual int write(const QCanBusFrame& dataFrame) = 0;

private:
    Q_DISABLE_COPY_MOVE(BaseCdeviceCan);

};

#endif // BASECDEVICECAN_H
