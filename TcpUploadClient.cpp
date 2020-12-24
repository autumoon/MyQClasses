//author：autumoon
//联系QQ：4589968
//日期：2020-10-20
#include "TcpUploadClient.h"
#include <QTextCodec>
#include <QDataStream>
#include <QEventLoop>
#include <QTimer>
#include "Log.h"

TcpUploadClient::TcpUploadClient(QObject *parent) : QObject(parent)
{
    //界面相关
    m_leHost = nullptr;
    m_lePort = nullptr;
    m_lbStatus = nullptr;

    m_nPort = 9999;

    m_tcpClient = nullptr;
    m_localFile = nullptr;
    m_totalBytes = 0;
    m_bytesWritten = 0;
    m_bytesToWrite = 0;

    //每次传输量
    m_payloadSize = 64 * 1024;

    //传输多个文件
    m_nFileIndex = 0;

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
}

bool TcpUploadClient::IsHostOnline(QString strHostName, quint16 nPort, int nTimeoutmseconds)
{
    QTcpSocket tcpClient;
     tcpClient.abort();
     if (strHostName.contains("http"))
     {
         strHostName= strHostName.right(strHostName.length() - strHostName.indexOf("://") - 3) ;
         if (strHostName.contains("/"))
         {
             strHostName = strHostName.left(strHostName.indexOf("/"));
         }
     }
     tcpClient.connectToHost(strHostName, nPort);
     //2000毫秒没有连接上则判断不在线
     return tcpClient.waitForConnected(nTimeoutmseconds);
}

void TcpUploadClient::initialize()
{
    m_tcpClient = new QTcpSocket(this);

    m_localFile = nullptr;
    m_totalBytes = 0;
    m_bytesWritten = 0;
    m_bytesToWrite = 0;

    //传输多个文件
    m_nFileIndex = 0;

    connect(m_tcpClient, SIGNAL(connected()), this, SLOT(send()));                                      //当连接成功时，就开始传送文件
    connect(m_tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(goOnSend(qint64)));
}

void TcpUploadClient::release()
{
    if (m_tcpClient)
    {
        disconnect(m_tcpClient, SIGNAL(connected()), this, SLOT(send()));
        disconnect(m_tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(goOnSend(qint64)));
        m_tcpClient->close();
        m_tcpClient->deleteLater();
    }
}

void TcpUploadClient::StartUpload(const QString &strLoaclPath, const QString &strServerFilePath)
{
    m_lLocalPaths.clear();
    m_lLocalPaths.push_back(strLoaclPath);
    m_lServerFilePaths.clear();
    m_lServerFilePaths.push_back(strServerFilePath);

    initialize();
    if (setUploadFilePath(m_lLocalPaths[m_nFileIndex], m_lServerFilePaths[m_nFileIndex]))
    {
        m_tcpClient->connectToHost(m_strHost, m_nPort);
    }
}

void TcpUploadClient::StartUpload(const QStringList &lLocalPaths, const QStringList &lServerFilePaths)
{
    m_lLocalPaths = lLocalPaths;
    m_lServerFilePaths = lServerFilePaths;

    initialize();
    if (setUploadFilePath(m_lLocalPaths[m_nFileIndex], m_lServerFilePaths[m_nFileIndex]))
    {
        m_tcpClient->connectToHost(m_strHost, m_nPort);
    }
}

void TcpUploadClient::send()
{
    if (m_localFile == nullptr || m_tcpClient == nullptr)
    {
        return;
    }

    if (m_lbStatus)
    {
        m_timer.restart();
    }

    m_bytesToWrite = m_localFile->size();  //剩余数据的大小
    m_totalBytes = m_bytesToWrite;

    QDataStream out(&m_outBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_6);

    out<<qint64(0)<<qint64(0)<<m_strServerFilePath;

    m_totalBytes += m_outBlock.size();  //总大小为文件大小加上文件名等信息大小
    m_bytesToWrite += m_outBlock.size();

    out.device()->seek(0);  //回到字节流起点来写好前面连个qint64，分别为总大小和文件名等信息大小
    out<<m_totalBytes<<qint64(m_outBlock.size());

    m_tcpClient->write(m_outBlock);  //将读到的文件发送到套接字

    if (m_lbStatus)
    {
        m_lbStatus->setText("已连接");
    }

    m_outBlock.clear();
}

void TcpUploadClient::goOnSend(qint64 numBytes)
{
    m_bytesToWrite -= numBytes;  //剩余数据大小
    m_outBlock = m_localFile->read(qMin(m_bytesToWrite, m_payloadSize));
    m_tcpClient->write(m_outBlock);

    if(m_bytesToWrite == 0)  //发送完毕
    {
        if (m_lbStatus)
        {
            QString strInfo = QString("传送文件 %1 成功").arg(m_filePath);
            m_lbStatus->setText(strInfo);
            CLOG::Out(strInfo);
        }

        m_localFile->close();
        m_localFile = nullptr;

        int nFileCount = m_lLocalPaths.size();

        //传输进度
        emit progress(m_nFileIndex + 1, nFileCount);
        emit finished_file(true, m_filePath  + " -> " +  m_strHost);

        if (++m_nFileIndex < nFileCount)
        {
            if (setUploadFilePath(m_lLocalPaths[m_nFileIndex], m_lServerFilePaths[m_nFileIndex]))
            {
                send();
            }
        }
        else
        {
            finishedAll();
        }
    }

    //发送进度
    if (m_lbStatus)
    {
        float useTime = m_timer.elapsed();
        double speed = m_bytesWritten / useTime;

        m_lbStatus->setText(tr("已上传 %1MB (%2MB/s) \n共%3MB 已用时:%4秒\n估计剩余时间：%5秒")
                                       .arg(m_bytesWritten / (1024*1024))//已上传
                                       .arg(speed*1000/(1024*1024),0,'f',2)//速度
                                       .arg(m_totalBytes / (1024 * 1024))//总大小
                                       .arg(useTime/1000,0,'f',0)//用时
                                       .arg(m_totalBytes/speed/1000 - useTime/1000,0,'f',0));//剩余时间
    }
    emit progress_file(m_bytesWritten, m_totalBytes);
}

bool TcpUploadClient::setUploadFilePath(const QString& strFilePath, const QString& strServerPath)
{
    m_bytesToWrite = 0;
    m_totalBytes = 0;
    m_outBlock.clear();

    m_filePath = strFilePath;
    m_strServerFilePath = strServerPath;
    m_localFile = new QFile(m_filePath);

    if(!m_localFile->open(QFile::ReadOnly))
    {
        CLOG::Out("open file %s error!", strFilePath.toUtf8().data());
        return  false;
    }

    CLOG::Out("正在传输:%s", strFilePath.toUtf8().data());

    return true;
}

void TcpUploadClient::sleep(int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

void TcpUploadClient::displayError(QAbstractSocket::SocketError)
{
    CLOG::Out(m_tcpClient->errorString());
    m_tcpClient->close();
    emit finished(false, m_strHost);

    if (m_lbStatus)
    {
        m_lbStatus->setText("上次传输出现错误！客户端重新就绪！");
    }
}

void TcpUploadClient::finishedAll()
{
    release();
    emit finished(true, m_strHost);
}
