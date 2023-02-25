#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>

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

    void on_connectBtn_clicked();

    void slotTimer();

    void slotTimerCheckConnection();

    void on_setTimer_clicked();

public slots:
    void slotReadyRead();

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QByteArray Data;    
    quint16 nextBlockSize;
    QTimer *timer;
    QTimer *timerCheckConnection;
    bool isConnected;

    void sendToServer(QString str);
    void setDiskInfo(QMap<int, QList<QString>> diskInfo);
    void checkConnection();
    void setIpValidator();


};
#endif // MAINWINDOW_H
