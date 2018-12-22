#include "mainchatwindow.h"
#include "ui_mainchatwindow.h"

MainChatWindow::MainChatWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainChatWindow)
{
    ui->setupUi(this);
    initMain();
}

MainChatWindow::~MainChatWindow()
{
    delete ui;

    if (m_serialPortWorker) delete m_serialPortWorker;
}

void MainChatWindow::initSerialPort()
{
    auto serialPort = new QSerialPort;

    serialPort->setPortName("COM1");
    serialPort->setBaudRate(QSerialPort::Baud115200);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setDataBits(QSerialPort::Data8);

    if (!serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << QObject::tr("Failed to open port %1 for R/W, error: %2")
            .arg(serialPort->portName())
            .arg(serialPort->errorString());
    }

    m_serialPortWorker = new SerialPortWorker(serialPort);
    connect(m_serialPortWorker,
            SIGNAL(dataReady()),
            this,
            SLOT(handleDataReady()));

    ui->statusBar->showMessage("Baudrate: "
                               +QString::number(serialPort->baudRate()));
}

void MainChatWindow::connectSignals()
{
    connect(ui->sendButton, SIGNAL(clicked()),
            this, SLOT(handlerSendButton()));
    connect(ui->inputField, SIGNAL(returnPressed()),
            ui->sendButton, SIGNAL(clicked()));
    connect(ui->crashBit, &QCheckBox::stateChanged,
            m_serialPortWorker, &SerialPortWorker::handleCrashBit);
    connect(ui->collision, &QCheckBox::stateChanged,
            m_serialPortWorker, &SerialPortWorker::handleCollision);
    connect(ui->isSolvedCollision, &QCheckBox::stateChanged,
            m_serialPortWorker, &SerialPortWorker::handleIsSolvedCollision);
    connect(ui->collision, &QCheckBox::stateChanged,
            this, &MainChatWindow::handleCollision);
}

void MainChatWindow::initMain()
{
    initSerialPort();
    createActions();
    connectSignals();
}

void MainChatWindow::createActions()
{
    connect(ui->mBaudrateAction, &QAction::triggered,
            this, &MainChatWindow::handleMenuBaudRate);
    connect(ui->mDisplayClear, &QAction::triggered,
            ui->textDisplay, &QTextBrowser::clear);
}

void MainChatWindow::handlerSendButton()
{
    if (ui->inputField->text().isEmpty()) return;

    QByteArray arr(ui->inputField->text().toUtf8());
    m_serialPortWorker->write(arr);
    bool collisionChecked = ui->collision->checkState() == Qt::Checked;
    bool isSolvedCollisionChecked = ui->isSolvedCollision->checkState() == Qt::Checked;
    if (!collisionChecked
        || (collisionChecked && isSolvedCollisionChecked))
        ui->textDisplay->append("-> " + ui->inputField->text());
    ui->inputField->clear();
}

void MainChatWindow::handleDataReady()
{
    ui->textDisplay->append("<- " + m_serialPortWorker->m_readData);
}

void MainChatWindow::handleMenuBaudRate()
{
    if (!m_serialPortWorker) return;

    DialogBaudrate dialog;

    connect(&dialog, SIGNAL(dataReady(qint32)), this,
            SLOT(handlerDialogDataReady(qint32)));

    dialog.exec();
}

void MainChatWindow::handlerDialogDataReady(qint32 data)
{
    if (data > 0 && m_serialPortWorker->changeBaudrate(data))
        ui->statusBar->showMessage("Baudrate: " + QString::number(data));
}

void MainChatWindow::handleCollision(int state)
{
    ui->isSolvedCollision->setEnabled(state == Qt::Checked);
}
