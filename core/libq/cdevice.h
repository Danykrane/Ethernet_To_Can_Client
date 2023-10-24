#ifndef CDEVICE_H
#define CDEVICE_H

#include <mutex>
#include <string>
#include <vector>
#include <functional>

#include <QtGlobal>

/*!
 * \brief Интерфейс класса для работы с внешним устройством
 */
class CDevice
{
    Q_DISABLE_COPY_MOVE(CDevice)
public:
    /*!
     * \brief Типы ошибок
     */
    enum ErrType
    {
        DEFAULT_ERROR = -1,
        SUCCESS       = 0
    };

    /*!
     * \brief Статус соединения
     */
    enum Status
    {
        disconnected,
        connected,
        error
    };

public:
    explicit CDevice(const std::string& addr);

    /*!
     * \brief Перегруженный декструктор должен вызывать close()
     */
    virtual ~CDevice() = default;

    /*!
     * \brief Устанавливает соединение с устройством
     */
    int init();

    /*!
     * \brief Закрывает соединение с устройством
     */
    int close();

    /*!
     * \brief !hasError()
     */
    bool ok() const;

    /*!
     * \brief True, если есть ошибки
     */
    bool hasError() const;

    /*!
     * \brief Возвращает текст последней ошибки
     */
    std::string getLastError() const;

    /*!
     * \brief Устанавливает никальный номер устройства на схеме
     */
    void setNum(const std::string& newNum);

    /*!
     * \brief Уникальный номер устройства на схеме
     */
    std::string num() const;

    /*!
     * \brief Устанавливает описание устройства для отображения
     */
    void setTitle(const std::string& newTitle);

    /*!
     * \brief Описание устройства для отображения
     */
    std::string title() const;

    /*!
     * \brief Устанавливает модель устройства для DeviceManager
     */
    void setModel(const std::string& newModel);

    /*!
     * \brief Модель устройства для DeviceManager
     */
    std::string model() const;

    /*!
     * \brief Адрес устройства
     */
    std::string address() const;

    /*!
     * \brief Текущий статус соединения
     */
    Status status() const;

    /*!
     * \brief Status == CDevice::connected
     */
    bool isConnected() const;


public:
    static int invokeWhileSuccess(std::vector<std::function<int()>> functions);
    static bool
        executeWhileSuccess(std::vector<std::function<int()>> functions);

protected:
    /*!
     * \brief Пользовательская реализация метода установки соединения
     */
    virtual int onInit();

    /*!
     * \brief Пользовательская реализация метода закрытия соединения
     */
    virtual int onClose();

    /*!
     * \brief Устанавливает текст последней ошибки
     */
    void setLastError(const std::string& err, int code);

private:
    /*!
     * \brief Отчищает ошибки и устанавливает статус Disconnected
     */
    void resetStatus();

private:
    using mutex_type = std::recursive_mutex;
    mutable mutex_type   m_mutex;

    Status               m_status;

    std::string          m_num;
    std::string          m_title;
    std::string          m_model;
    std::string          m_address;

    std::string          m_error;
};

#endif
