#ifndef CDEVICEUSRCANET200_H
#define CDEVICEUSRCANET200_H

#include "basecdevicecan.h"
#include "qobjectdefs.h"

class QThread;
class CdeviceUsrCanet200Private;
/*!
 * \brief Класс устройства USR-Canet200
 */
class CdeviceUsrCanet200 : public BaseCdeviceCan
{
public:

    /*!
     * \brief CdeviceUsrCanet200 - Конструктор класса устройтсва
     * \param addr - адрес из БД ("IP:port")
     */
    explicit CdeviceUsrCanet200(const std::string& addr);

    /*!
     * \brief read - реализация чтения данных
     * \param dataFrame - данные
     * \return
     */
    int read(QByteArray& dataFrame) override;

    /*!
     * \brief write - реализация записи данных
     * \param dataFrame - данные
     * \return
     */
    int write(const QCanBusFrame& dataFrame) override;

    /*!
     * \brief onInit - создать соединение
     * \return true - соединение разорвано успешно, false - проблема
     */
    int onInit() override;
    /*!
     * \brief onClose - разорвать соединение
     * \return true - соединение разорвано успешно, false - проблема
     */
    int onClose() override;

    /*!
     * \brief isConnected - текущий статус соединений
     * \return true - соединение установлено, false - проблема с соединением
     */
    bool isConnected() override;

public:
    /*!
     * \brief setHostName -установить ip хоста
     * \param hostName - ip хоста
     */
    void setHostName(const QString& hostName);

    /*!
     * \brief setPort - установить номер порта
     * \param port - номер порта
     */
    void setPort(uint16_t port);

    /*!
     * \brief hostName - получить IP хоста
     */
    [[nodiscard]] const QString hostName();

    /*!
     * \brief port - получить номер порта
     */
    [[nodiscard]] uint16_t port();

    /*!
     * \brief setWaitConnectTime - задать время ожидания
     * \param waitMsec - время ожидания
     */
    void setWaitConnectTime(uint16_t waitMsec);

    /*!
     * \brief waitMsec - Получить времня ожидания соединения
     * \return
     */
    [[nodiscard]] const uint16_t waitMsec();

//signals:
//    void sendedFromSocketData();

private:
    Q_DECLARE_PRIVATE(CdeviceUsrCanet200);
    std::unique_ptr<QThread> m_thread;
    std::unique_ptr<CdeviceUsrCanet200Private> d_ptr;
};

#endif // CDEVICEUSRCANET200_H
