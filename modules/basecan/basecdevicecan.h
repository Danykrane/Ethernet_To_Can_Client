#ifndef BASECDEVICECAN_H
#define BASECDEVICECAN_H

#include "cdevice.h"

class BaseCdeviceCan : public CDevice
{
public:
    BaseCdeviceCan(const std::string& addr);

protected:
    /*!
    * \brief Пользовательская реализация метода установки соединения
//    */
//    virtual int onInit();

//    /*!
//     * \brief Пользовательская реализация метода закрытия соединения
//     */
//    virtual int onClose();
};

#endif // BASECDEVICECAN_H
