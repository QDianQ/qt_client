#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QStorageInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    nextBlockSize = 0;

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


    timer = new QTimer(this);
    timerCheckConnection = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    connect(timerCheckConnection, SIGNAL(timeout()), this, SLOT(slotTimerCheckConnection()));
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_connectBtn_clicked()
{


    QString input_ip = ui->input_IP->text();
    QString input_port = ui->input_port->text();

    socket->connectToHost(input_ip, input_port.toInt());

    if (QAbstractSocket::ConnectedState == socket->state())
    {
        qDebug() << "Connected";
    }
    sendToServer("_getDiskInfo");

    timerCheckConnection->setInterval(1000);
    timerCheckConnection->start();

    timer->setInterval(ui->doubleSpinBox->value() * 1000);
    timer->start();
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
//                qDebug() << "index: " << i
//                         << "column" << id_column
//                         << "item: " << j;

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
    if (QAbstractSocket::UnconnectedState == socket->state())
    {

        QMessageBox::warning(this, "Warning!", "Client was disconnected.");
        timerCheckConnection->stop();
    }
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
void MainWindow::on_disconnectBtn_clicked()
{
    timer->stop();
    timerCheckConnection->stop();
    socket->disconnectFromHost();

    qDebug() << "client was disconnected";
}
void MainWindow::slotTimer()
{
    sendToServer("_getDiskInfo");
}
void MainWindow::slotTimerCheckConnection()
{
    checkConnection();
}
void MainWindow::on_setTimer_clicked()
{
    timer->setInterval(ui->doubleSpinBox->value() * 1000);
}

