#ifndef SERIALPORTWORKER_H
#define SERIALPORTWORKER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QCoreApplication>
#include <QTextStream>
#include <QMessageBox>

#include <iostream>

class SerialPortWorker : public QObject {
    Q_OBJECT

public:

    explicit SerialPortWorker(QSerialPort *serialPort,
                              QObject     *parent = nullptr);
    ~SerialPortWorker();

    void write(const QByteArray& writeData);
    bool changeBaudrate(const qint32& value);

    QByteArray m_readData;

signals:

    void dataReady();

private slots:

    void handleBytesWritten(qint64 bytes);
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:

    QSerialPort *m_serialPort = nullptr;
    QByteArray m_writeData;
    qint64 m_bytesWritten = 0;
    QTextStream m_standardOutput;
};

#endif // SERIALPORTWORKER_H
