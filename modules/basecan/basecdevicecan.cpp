#include "basecdevicecan.h"

/* ---------------------------- BaseCdeviceCan ----------------------------- */
BaseCdeviceCan::BaseCdeviceCan(const std::string &addr): CDevice(addr)
{

}

int BaseCdeviceCan::onInit()
{
    return (connect() == true)? CDevice::SUCCESS: CDevice::error;
}

int BaseCdeviceCan::onClose()
{
    disconnect();
    return CDevice::SUCCESS;
}
