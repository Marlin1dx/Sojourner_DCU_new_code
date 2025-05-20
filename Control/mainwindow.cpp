#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serialPort(new QSerialPort(this))
    , readTimer(new QTimer(this))
    , roverView(new RoverViewWidget(this))
{
    ui->setupUi(this);
    
    // Добавляем виджет марсохода в layout
    ui->verticalLayout->addWidget(roverView);
    
    // Настраиваем таймер для чтения данных
    connect(readTimer, &QTimer::timeout, this, &MainWindow::readData);
    readTimer->start(100); // Чтение каждые 100 мс
    
    // Настраиваем серийный порт
    setupSerialPort();
    
    // Заполняем список доступных портов
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        ui->portComboBox->addItem(info.portName());
    }
}

MainWindow::~MainWindow()
{
    if (serialPort->isOpen())
        serialPort->close();
    delete ui;
}

void MainWindow::setupSerialPort()
{
    connect(serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
}

void MainWindow::on_connectButton_clicked()
{
    serialPort->setPortName(ui->portComboBox->currentText());
    serialPort->setBaudRate(QSerialPort::Baud115200);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (serialPort->open(QIODevice::ReadWrite)) {
        ui->connectButton->setEnabled(false);
        ui->disconnectButton->setEnabled(true);
        ui->portComboBox->setEnabled(false);
    } else {
        QMessageBox::critical(this, tr("Error"), serialPort->errorString());
    }
}

void MainWindow::on_disconnectButton_clicked()
{
    if (serialPort->isOpen())
        serialPort->close();
    ui->connectButton->setEnabled(true);
    ui->disconnectButton->setEnabled(false);
    ui->portComboBox->setEnabled(true);
}

void MainWindow::on_sendButton_clicked()
{
    if (serialPort->isOpen()) {
        QByteArray data = ui->commandLineEdit->text().toUtf8() + "\n";
        serialPort->write(data);
    }
}

void MainWindow::readData()
{
    if (serialPort->isOpen() && serialPort->bytesAvailable()) {
        QByteArray data = serialPort->readAll();
        updateRoverView(data);
    }
}

void MainWindow::updateRoverView(const QByteArray &data)
{
    // Парсим данные телеметрии
    QString str = QString::fromUtf8(data);
    if (str.contains("HALL:")) {
        QString hallData = str.section("HALL:", 1, 1).section("|", 0, 0);
        QStringList angles = hallData.split(",");
        if (angles.size() == 4) {
            float alf = angles[0].toFloat();
            float alr = angles[1].toFloat();
            float arf = angles[2].toFloat();
            float arr = angles[3].toFloat();
            
            // Обновляем виджет марсохода
            roverView->updateWheelAngles(alf, alr, arf, arr);
        }
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serialPort->errorString());
        on_disconnectButton_clicked();
    }
} 