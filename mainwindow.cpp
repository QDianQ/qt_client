#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QStorageInfo>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    nextBlockSize = 0;
    isConnected = false;

    QTreeWidgetItem *treeHeader = new QTreeWidgetItem();

    treeHeader->setText(0, "Disk letter");
    treeHeader->setText(1, "Disk name");
    treeHeader->setText(2, "Total size");
    treeHeader->setText(3, "Using");
    treeHeader->setText(4, "Free space");

    ui->treeWidget->setHeaderItem(treeHeader);
    ui->doubleSpinBox->setMinimum(0.01);
    ui->doubleSpinBox->setMaximum(10.00);
    ui->doubleSpinBox->setValue(10.00);

    setIpValidator();

    timer = new QTimer(this);
    timerCheckConnection = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    connect(timerCheckConnection, SIGNAL(timeout()), this, SLOT(slotTimerCheckConnection()));
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::setIpValidator()
{
    QString ipRange = "(([ 0]+)|([ 0]*[0-9] *)|([0-9][0-9] )|([ 0][0-9][0-9])|(1[0-9][0-9])|([2][0-4][0-9])|(25[0-5]))";
    QRegularExpression ipRegex ("^" + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange + "$");
    QRegularExpressionValidator *ipValidator = new QRegularExpressionValidator(ipRegex, this);
    ui->input_IP->setValidator(ipValidator);
    ui->input_IP->setInputMask("000.000.000.000");
}
void MainWindow::on_connectBtn_clicked()
{
    if (!isConnected)
    {
        QString input_ip = ui->input_IP->text();
        QString input_port = ui->input_port->text();

        socket = new QTcpSocket(this);
        connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

        socket->connectToHost(input_ip, input_port.toInt());

        sendToServer("_getDiskInfo");

        timerCheckConnection->setInterval(1000);
        timerCheckConnection->start();

        timer->setInterval(ui->doubleSpinBox->value() * 1000);
        timer->start();
        ui->connectBtn->setText("Disconnect");
        ui->statusConnection->setText("Status: connected");
        isConnected = true;
    }
    else
    {
        timer->stop();
        timerCheckConnection->stop();
        socket->disconnectFromHost();

        ui->connectBtn->setText("Connect");
        ui->statusConnection->setText("Status: disconnected");
        isConnected = false;
    }

}
void MainWindow::sendToServer(QString str)
{
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_4);
    out << quint16(0) << str;
    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));
    socket->write(Data);
}
void MainWindow::setDiskInfo(QMap<int, QList<QString> > diskInfo)
{
    QTreeWidgetItem *treeItems;
    if(ui->treeWidget->topLevelItemCount() <= 0)
    {
        int id_column = 0;
        for (int i : diskInfo.keys())
        {
            treeItems = new QTreeWidgetItem();
            id_column = 0;
            for (QString j : diskInfo[i])
            {
                treeItems->setText(id_column, j);
                id_column++;
            }
            ui->treeWidget->addTopLevelItem(treeItems);
        }
    }
    else
    {
        int id_column = 0;
        for (int i : diskInfo.keys())
        {
            id_column = 0;
            for (QString j : diskInfo[i])
            {
                ui->treeWidget->topLevelItem(i)->setText(id_column, j);
                id_column++;
            }
        }
    }

}
void MainWindow::checkConnection()
{

}
void MainWindow::slotReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_4);
    if(in.status() == QDataStream::Ok)
    {
        for(;;)
        {
            if(nextBlockSize == 0)
            {
                if(socket->bytesAvailable() < 2)
                {
                    break;
                }
                in >> nextBlockSize;
            }

            if(socket->bytesAvailable() < nextBlockSize)
            {
                break;
            }

            QMap<int, QList<QString>> diskInfo;

            in >> diskInfo;
            nextBlockSize = 0;

            setDiskInfo(diskInfo);
        }
    }
    else{
        qDebug() << ("read error");
    }
}
void MainWindow::slotTimer()
{
    if (QAbstractSocket::ConnectedState == socket->state())
    {
        sendToServer("_getDiskInfo");
    }
    else
    {
        timer->stop();
    }
}
void MainWindow::slotTimerCheckConnection()
{
    if (QAbstractSocket::UnconnectedState == socket->state())
    {

        timer->stop();
        timerCheckConnection->stop();

        socket->disconnectFromHost();
        ui->connectBtn->setText("Connect");
        ui->statusConnection->setText("Status: disconnected");
        isConnected = false;

        QMessageBox::warning(this, "Warning!", "Client was disconnected.");

    }
}
void MainWindow::on_setTimer_clicked()
{
    timer->setInterval(ui->doubleSpinBox->value() * 1000);
}

