//author：autumoon
//联系QQ：4589968
//日期：2020-10-20
#ifndef TCPUPLOADSERVER_H
#define TCPUPLOADSERVER_H

#include <QObject>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QFile>
#include <QLabel>
#include <QElapsedTimer>

class TcpUploadServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpUploadServer(QObject *parent = nullptr);

    bool StartServer();
    void SetLableStatus(QLabel *lableStatus){m_lbStatus = lableStatus;}
    void SetPort(const quint16& nPort){m_nPort = nPort;}

signals:
    void begin();
    void progress(qint64, qint64);
    void finished(bool bSuccess);

private slots:
    void acceptConnection();
    void readClient();
    void displayError(QAbstractSocket::SocketError socketError);

private:
    void initialize();
    void release();

private:
    //界面相关
    QLabel *m_lbStatus;

    quint16 m_nPort;
    QElapsedTimer m_timer;

    bool m_busy;

    QTcpServer *m_tcpServer;
    QTcpSocket *m_tcpReceivedSocket;
    qint64 m_totalBytes;
    qint64 m_bytesReceived;
    qint64 m_filePathSize;
    QString m_filePathName;
    QFile *m_localFile;
    QByteArray m_inBlock;
};

#endif // TCPUPLOADSERVER_H
