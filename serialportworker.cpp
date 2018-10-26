#include "serialportworker.h"

SerialPortWorker::SerialPortWorker(QSerialPort *serialPort,
                                   QObject     *parent) :
    QObject(parent),
    m_serialPort(serialPort),
    m_standardOutput(stdout)
{
    connect(m_serialPort, &QSerialPort::readyRead,     this,
            &SerialPortWorker::handleReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this,
            &SerialPortWorker::handleError);
    connect(m_serialPort, &QSerialPort::bytesWritten,  this,
            &SerialPortWorker::handleBytesWritten);
}

SerialPortWorker::~SerialPortWorker()
{
    if (m_serialPort)
    {
        if (m_serialPort->isOpen()) m_serialPort->close();
        delete m_serialPort;
    }
}

void SerialPortWorker::handleBytesWritten(qint64 bytes)
{
    m_bytesWritten += bytes;

    if (m_bytesWritten == m_writeData.size())
    {
        m_bytesWritten = 0;
        m_standardOutput << QObject::tr(
            "Data successfully sent to port %1")
            .arg(m_serialPort->portName())
                         << endl;
    }
}

void SerialPortWorker::handleReadyRead()
{
    m_readData = m_serialPort->readAll();

    if (m_readData.isEmpty())
    {
        m_standardOutput << QObject::tr(
            "No data was currently available "
            "for reading from port %1")
            .arg(m_serialPort->portName())
                         << endl;
    }
    else
    {
        m_standardOutput << QObject::tr(
            "Data successfully received from port %1")
            .arg(m_serialPort->portName())
                         << endl;
        emit dataReady();
    }
}

void SerialPortWorker::handleError(QSerialPort::SerialPortError error)
{
    QString errorTextTemplate;

    switch (error) {
      case QSerialPort::ReadError:
          errorTextTemplate = "An I/O error occurred while reading "
                              "the data from port %1, error: %2";
          break;
      case QSerialPort::WriteError:
          errorTextTemplate = "An I/O error occurred while writing"
                              " the data to port %1, error: %2";
          break;
      default: break;
    }

    QMessageBox::critical(nullptr, "Warning!",
                          QObject::tr(errorTextTemplate.toUtf8())
                          .arg(m_serialPort->portName())
                          .arg(m_serialPort->errorString()));

    QCoreApplication::exit(1);
}

void SerialPortWorker::write(const QByteArray& writeData)
{
    m_writeData = writeData;

    const qint64 bytesWritten = m_serialPort->write(m_writeData.constData(),
                                                    m_writeData.size());

    if (bytesWritten == -1) {
        m_standardOutput << QObject::tr(
            "Failed to write the data to port %1, error: %2")
            .arg(m_serialPort->portName())
            .arg(m_serialPort->errorString())
                         << endl;
    } else if (bytesWritten != m_writeData.size()) {
        m_standardOutput << QObject::tr(
            "Failed to write Rall the data to port %1, error: %2")
            .arg(m_serialPort->portName())
            .arg(m_serialPort->errorString())
                         << endl;
    }
}

bool SerialPortWorker::changeBaudrate(const qint32& value)
{
    if (m_serialPort) return m_serialPort->setBaudRate(value);
    return false;
}
