//author：autumoon
//联系QQ：4589968
//日期：2020-10-20
#ifndef TCPUPLOADCLIENT_H
#define TCPUPLOADCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QElapsedTimer>

class TcpUploadClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpUploadClient(QObject *parent = nullptr);

    void SetLableStatus(QLabel *lableStatus){m_lbStatus = lableStatus;}
    void SetHostAndPort(const QString& strHost, const quint16& nPort){m_strHost = strHost; m_nPort = nPort;}
    //需要用QLineEdit显示主机和端口调用
    void SetLineEditHostAndPort(QLineEdit *leHost, QLineEdit *lePort){m_leHost = leHost; m_lePort = lePort;}

public:
    void StartUpload(const QString& strLoaclPath, const QString& strServerFilePath);
    //vServerFilePaths 用于指定要存储到服务器上的全路径
    void StartUpload(const QStringList& lLocalPaths, const QStringList& lServerFilePaths);

    //判断主机是否在线
    static bool IsHostOnline(QString strHostName, quint16 nPort = 80, int nTimeoutmseconds = 2000);
    //判断是否通外网，能连接百度IP说明可以通外网
    static bool IsWebOk(){return IsHostOnline("202.108.22.5", 80);}

signals:
    void progress(qint64, qint64);
    void progress_file(qint64, qint64);
    //成功与否，主机IP
    void finished(bool, const QString&);
    //成功与否，文件全路径
    void finished_file(bool, const QString&);

private slots:
    void displayError(QAbstractSocket::SocketError);
    void goOnSend(qint64);                                          //传送文件内容
    void send();                                                              //传送文件头信息

private:
    bool setUploadFilePath(const QString& strFilePath, const QString& strServerPath);

    void finishedAll();                                                    //全部文件传输完成
    void initialize();
    void release();
    void sleep(int msec);

private:
    //界面相关
    QLineEdit *m_leHost;
    QLineEdit *m_lePort;
    QLabel *m_lbStatus;
    QElapsedTimer m_timer;

    QString m_strHost;
    quint16 m_nPort;

    QTcpSocket *m_tcpClient;
    QFile *m_localFile;
    qint64 m_totalBytes;
    qint64 m_bytesWritten;
    qint64 m_bytesToWrite;
    qint64 m_payloadSize;
    QString m_filePath;
    QString m_strServerFilePath;
    QByteArray m_outBlock;

    //传输多个文件
    int m_nFileIndex;
    QStringList m_lLocalPaths;
    QStringList m_lServerFilePaths;
};

#endif // TCPUPLOADCLIENT_H
