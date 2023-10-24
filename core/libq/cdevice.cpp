#include "cdevice.h"

#include <sstream>



CDevice::CDevice(const std::string &addr):
    m_address(addr)
{
    resetStatus();
}

bool CDevice::ok() const
{
    return !hasError();
}


int CDevice::init()
{
    const std::lock_guard<mutex_type> lock(m_mutex);

    //    if (!m_deviceManager)
//    {
//        setLastError("DeviceManager required", CDevice::DEFAULT_ERROR);
//        return CDevice::DEFAULT_ERROR;
//    }

    if (m_status == connected && !hasError())
    {
        return CDevice::SUCCESS;
    }

    long res = onInit();
    if (res == CDevice::SUCCESS)
    {
        m_status = connected;
    }
    else
    {
        m_status = error;
    }

#ifdef ENABLE_CDEVICE_MOCK
    m_status = connected;
#endif

    return res;
}

int CDevice::onInit()
{
    return -1;
}

int CDevice::onClose()
{
    return -1;
}

int CDevice::close()
{
    const std::lock_guard<mutex_type> lock(m_mutex);

    if (status() == disconnected)
    {
        return CDevice::SUCCESS;
    }

    resetStatus();

    return onClose();
}

bool CDevice::hasError() const
{
#ifdef ENABLE_CDEVICE_MOCK
    return false;
#endif

    const std::lock_guard<mutex_type> lock(m_mutex);
    return m_status == error;
}

std::string CDevice::address() const
{
    const std::lock_guard<mutex_type> lock(m_mutex);
    return m_address;
}

CDevice::Status CDevice::status() const
{
#ifdef ENABLE_CDEVICE_MOCK
    return connected;
#endif

    const std::lock_guard<mutex_type> lock(m_mutex);
    return m_status;
}

bool CDevice::isConnected() const
{
    return m_status == CDevice::connected;
}

std::string CDevice::getLastError() const
{
    const std::lock_guard<mutex_type> lock(m_mutex);
    return m_error;
}

void CDevice::resetStatus()
{
    m_status = disconnected;
    m_error  = "No errors";
}

std::string CDevice::num() const
{
    const std::lock_guard<mutex_type> lock(m_mutex);
    return m_num;
}

std::string CDevice::title() const
{
    const std::lock_guard<mutex_type> lock(m_mutex);
    return m_title;
}

std::string CDevice::model() const
{
    const std::lock_guard<mutex_type> lock(m_mutex);
    return m_model;
}

void CDevice::setLastError(const std::string& err, int code)
{
    const std::lock_guard<mutex_type> lock(m_mutex);
#ifdef ENABLE_CDEVICE_MOCK
    return;
#endif

    std::string       buf;
    std::stringstream str(buf);

    str << "Number: " << m_num << "\n";
    str << "Address: " << m_address << "\n";
    str << "Error code: " << code << "\n";
    str << "Error text: " << err;

    m_error = str.str();

    m_status = error;

//    if (deviceManager())
//    {
//        deviceManager()->updateStatus();
//    }
}

//DeviceManager* CDevice::deviceManager() const
//{
//    const std::lock_guard<mutex_type> lock(m_mutex);
//    return m_deviceManager;
//}

int CDevice::invokeWhileSuccess(std::vector<std::function<int()>> functions)
{
    for (auto f: functions)
    {
        if (int res = f(); res != CDevice::SUCCESS)
        {
            return res;
        }
    }

    return CDevice::SUCCESS;
}

bool CDevice::executeWhileSuccess(std::vector<std::function<int()>> functions)
{
    return invokeWhileSuccess(functions) == CDevice::SUCCESS;
}

void CDevice::setModel(const std::string& newModel)
{
    const std::lock_guard<mutex_type> lock(m_mutex);
    m_model = newModel;
}

void CDevice::setTitle(const std::string& newTitle)
{
    const std::lock_guard<mutex_type> lock(m_mutex);
    m_title = newTitle;
}

void CDevice::setNum(const std::string& newNum)
{
    const std::lock_guard<mutex_type> lock(m_mutex);
    m_num = newNum;
}
