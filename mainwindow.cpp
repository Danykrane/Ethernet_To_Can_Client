#include "mainwindow.h"
#include "Client/tcpclient.h"
#include "utils/hexspinbox.cpp"

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <QVector>
#include <QMap>

#include <QFontMetrics>

#include <QCanBusFrame>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // визуальное  отображение
    QVBoxLayout *vertLayer = new QVBoxLayout(new QWidget);

    //----------------------------------------------------- подключение к хосту
    auto *gridConnection = new QGridLayout(new QWidget);
    auto *hostLabel = new QLabel("host name");
    auto *portLabel = new QLabel("port");
    auto *hostInput = new QLineEdit("192.168.1.17");

    auto *portInput = new QSpinBox;
    portInput->setRange(0, 65535);
    portInput->setValue(20001);

    auto *connectBtn = new QPushButton("connect");
    auto *disconnectBtn = new QPushButton("disconnect");
    disconnectBtn->setVisible(false);
    gridConnection->addWidget(hostLabel, 0, 0);
    gridConnection->addWidget(hostInput, 1, 0);
    gridConnection->addWidget(portLabel, 0, 1);
    gridConnection->addWidget(portInput, 1, 1);
    gridConnection->addWidget(disconnectBtn, 2, 1);

    vertLayer->addWidget(gridConnection->parentWidget());
    vertLayer->addWidget(connectBtn, 0, Qt::AlignRight);

    //--------------------------------------------------------- отправка данных

    QLabel *inputText = new QLabel("Can data frame");

    // Данные для ввода

    //    QLineEdit*   writeData    = new QLineEdit;
    QPushButton* sendBtn      = new QPushButton("send");
    QLabel *getDataText = new QLabel("Recieved data");
    QTextEdit*   recievedData = new QTextEdit;
    QPushButton* clearFormBtn = new QPushButton("clear");

    // --------------------------------------------------------------- Кадр Can
    QGridLayout* gridInput = new QGridLayout(new QWidget);

    // length
    QLabel*     lengthLabel = new QLabel("Length");
    HexSpinBox* lenSpinBx   = new HexSpinBox(true, this);

    lenSpinBx->setMinimumWidth(lengthLabel->width() / 8);
    lenSpinBx->setRange(0, 8);
    gridInput->addWidget(lengthLabel, 0, 0, Qt::AlignHCenter);
    gridInput->addWidget(lenSpinBx, 1, 0);

    // Can ID
    QLabel*     canIdLabel  = new QLabel("Can ID");
    HexSpinBox* canIdSpinBx = new HexSpinBox(true, this);

    canIdSpinBx->setMinimumWidth(lengthLabel->width() / 6);
    canIdSpinBx->setRange(0, 2048);
    gridInput->addWidget(canIdLabel, 0, 1, Qt::AlignHCenter);
    gridInput->addWidget(canIdSpinBx, 1, 1);

    // Frame data B7...B0
    uint16_t               colDataFrames = 8;
    QMap<int, HexSpinBox*> dataSpinBoxes;
    for (int i = 0; i < colDataFrames; ++i)
    {
        QLabel* dataLabel
            = new QLabel(QString("B%1").arg(colDataFrames - i - 1));
        HexSpinBox* dataSpinBx = new HexSpinBox(true, this);

        dataSpinBx->setRange(0, 255);
        dataSpinBx->setEnabled(false);

        dataSpinBoxes.insert(colDataFrames - i - 1, dataSpinBx);

        gridInput->addWidget(dataLabel, 0, 2 + i, Qt::AlignHCenter);
        gridInput->addWidget(dataSpinBx, 1, 2 + i);
    }

    // ------------------------------------------------------ alert message box

    connect(lenSpinBx, &HexSpinBox::textChanged, [lenSpinBx, dataSpinBoxes] () {
        //        qDebug() << lenSpinBx->hexValue();

        // отключим все
        for (int i = 0; i < dataSpinBoxes.size(); ++i)
        {
            dataSpinBoxes[i]->setEnabled(false);
        }

        // включим нужное
        for (int i = 0; i < lenSpinBx->value(); ++i)
        {
            dataSpinBoxes[dataSpinBoxes.size() - i - 1]->setEnabled(true);
        }
    });

    // ------------------------------------------------------------------------

    vertLayer->addWidget(inputText, 0, Qt::AlignHCenter);
    vertLayer->addWidget(gridInput->parentWidget());
    vertLayer->addWidget(sendBtn);
    vertLayer->addWidget(getDataText, 0, Qt::AlignHCenter);
    vertLayer->addWidget(recievedData);

    QHBoxLayout* horLayer = new QHBoxLayout(new QWidget);
    horLayer->addWidget(clearFormBtn, 0, Qt::AlignLeft);
    //    horLayer->addWidget(recieveDataBtn, 0, Qt::AlignRight);

    vertLayer->addWidget(horLayer->parentWidget());

    //================================================================== Клиент

    TcpClient* client = new TcpClient;

    connect(client, &TcpClient::connectedToServer, [] () {
        qDebug() << "Connected to server";
    });

    connect(client, &TcpClient::disconnectedFromServer, [] () {
        qDebug() << "Disconnected from server";
    });

    connect(client, &TcpClient::dataReceived, [=](const QByteArray &data) {
        qDebug() << "Received data:" << data;

        recievedData->append(data.toHex('/'));
    });

    connect(connectBtn, &QPushButton::clicked, [=]() {
        bool state = client->connectToServer(hostInput->text(), portInput->text().toInt());
        if (state) {
            hostInput->setEnabled(false);
            portInput->setEnabled(false);
            connectBtn->setVisible(false);
            disconnectBtn->setVisible(true);
        }
        else {
            // предупреждение
            QMessageBox::critical(this,"Message", "Wrong ip or port \nTry to ping", QMessageBox::Ok);
        }
    });

    connect(disconnectBtn, &QPushButton::clicked, [=]() {
        client->disconnectFromServer();

        hostInput->setEnabled(true);
        portInput->setEnabled(true);
        connectBtn->setVisible(true);
        disconnectBtn->setVisible(false);
    });

    //----------------------------------------------------- Коннект к хосту

    //    client->connectToServer("192.168.0.7", 20001);

    connect(sendBtn, &QPushButton::clicked, [=]() {
        QCanBusFrame frame;
        frame.setFrameId(canIdSpinBx->value());
        QByteArray data;
        for (int i = 0; i < dataSpinBoxes.size(); ++i) {
            if (dataSpinBoxes[i]->isEnabled()) {
                data.prepend(static_cast<char>(dataSpinBoxes[i]->value()));
            }
        }
        //        qDebug() << data;
        frame.setPayload(data);
        qDebug() << frame.toString();
        //        QString currDataText = writeData->text();
        //        writeData->clear();
        //        if (!currDataText.isEmpty())
        //        {
        //            QByteArray sendData = currDataText.toUtf8();
        //            client->sendData(sendData);
        //        }

        //         85 12 34 56 78 12 34 56 78 00 00 00 00

        // QByteArray data = QByteArray("\x81\x12"
        //                                     "4Vx\x01\x02\x03\x04\x05\x06\x07\x08");

        // client->sendData(data);
    });

    connect(clearFormBtn, &QPushButton::clicked, [=]() { recievedData->clear(); });

    //    connect(recieveDataBtn, )
    //    \201\0224Vx\001\002\003\004\005
    //    \205\0224Vx\001\002\003\004\005\000\000\000\205\0224Vx\001\002\003\004\005\000\000\000\205\0224Vx\001\002\003\004\005\000\000\000\205\0224Vx\001\002\003\004\005\000\000\000\205\0224Vx\001\002\003\004\005\000\000\000\205\0224Vx\001\002\003\004\005\000\000\000

    //=========================================================================

    setCentralWidget(vertLayer->parentWidget());

    // очищаем начальный экран дисплея
    //    clearFormBtn->click();
}

MainWindow::~MainWindow()
{
}
