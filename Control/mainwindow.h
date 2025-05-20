#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include "rover_view_widget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_sendButton_clicked();
    void readData();
    void handleError(QSerialPort::SerialPortError error);

private:
    Ui::MainWindow *ui;
    QSerialPort *serialPort;
    QTimer *readTimer;
    RoverViewWidget *roverView;

    void setupSerialPort();
    void updateRoverView(const QByteArray &data);
};

#endif // MAINWINDOW_H 