//author：autumoon
//联系QQ：4589968
//日期：2020-10-20
#include "TcpUploadServer.h"
#include <QTextCodec>
#include <QDataStream>
#include <QFileInfo>
#include "StdDirFile.h"
#include "Log.h"

TcpUploadServer::TcpUploadServer(QObject *parent) : QObject(parent)
{
    //界面相关
    m_lbStatus = nullptr;

    m_nPort = 9999;

    m_busy = false;

    m_tcpServer = nullptr;
    m_tcpReceivedSocket = nullptr;
    m_totalBytes = 0;
    m_bytesReceived = 0;
    m_filePathSize = 0;
    m_localFile = 0;

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
}

void TcpUploadServer::initialize()
{
    m_totalBytes = 0;
    m_bytesReceived = 0;
    m_filePathSize = 0;

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}

void TcpUploadServer::release()
{
    if (m_tcpReceivedSocket)
    {
        disconnect(m_tcpReceivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
        disconnect(m_tcpReceivedSocket, SIGNAL(error(QAbstractSocket::SocketError)), this ,SLOT(displayError(QAbstractSocket::SocketError)));
        m_tcpReceivedSocket->close();
        m_tcpReceivedSocket->deleteLater();
    }

    if (m_tcpServer)
    {
        disconnect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
        m_tcpServer->close();
        m_tcpServer->deleteLater();
    }
}

bool TcpUploadServer::StartServer()
{
    if (m_tcpServer)
    {
        release();
    }

    initialize();
    CLOG::Out("starting service...");

    if(!m_tcpServer->listen(QHostAddress("localhost"), m_nPort))
    {
        CLOG::Out("%s", m_tcpServer->errorString().toUtf8().data());

        return false;
    }

    QString strListen = "Listening port " + QString::number(m_nPort) + " !";
    CLOG::Out("%s", strListen.toUtf8().data());

    if (m_lbStatus)
    {
        m_lbStatus->setText("正在监听...");
    }

    return true ;
}

void TcpUploadServer::acceptConnection()
{
    if (m_busy)
    {
        CLOG::Out("TcpUploadServer is busy now!");
        return;
    }
    else
    {
        m_busy = true;
    }


    emit begin();

    if (m_lbStatus)
    {
        m_timer.restart();
    }

    m_tcpReceivedSocket = m_tcpServer->nextPendingConnection();
    connect(m_tcpReceivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(m_tcpReceivedSocket, SIGNAL(error(QAbstractSocket::SocketError)), this ,SLOT(displayError(QAbstractSocket::SocketError)));

    if (m_lbStatus)
    {
        m_lbStatus->setText("接受连接");
    }
}

void TcpUploadServer::readClient()
{
    if(m_bytesReceived<= sizeof(qint64)*2)  //才刚开始接收数据，此数据为文件信息
    {
        QDataStream in(m_tcpReceivedSocket);
        in.setVersion(QDataStream::Qt_5_6);
        //in>>m_totalBytes>>m_bytesReceived>>m_filePathName;

        if((m_tcpReceivedSocket->bytesAvailable()>=sizeof(qint64)*2)&&(m_filePathSize==0))
        {
            // 接收数据总大小信息和带路径的文件名大小信息
            in>>m_totalBytes>>m_filePathSize;
            m_bytesReceived +=sizeof(qint64)*2;
        }

        if((m_tcpReceivedSocket->bytesAvailable()>=m_filePathSize)&&(m_filePathSize!=0))
        {
            // 接收文件名，并建立文件
            in>>m_filePathName;

            //传输出现错误
            if (m_filePathName.length() == 0)
            {
                return;
            }

            //可能需要建立文件夹
            if (m_filePathName.indexOf("/")  != -1)
            {
                QString strFileName = CStdStr::GetNameOfFile(m_filePathName, '/');
                QString strSaveDir = CStdStr::GetDirOfFile(m_filePathName);
                QFileInfo fiDir(strSaveDir);
                if (!fiDir.exists()&& !CStdDir::createDirectory(strSaveDir))
                {
                    if (m_lbStatus)
                    {
                        m_lbStatus->setText(tr("接收文件 %1 失败！").arg(m_filePathName));
                    }
                    QString strFileFailed = (tr("接收文件 %1 失败！").arg(m_filePathName));
                    CLOG::Out("%s", strFileFailed.toUtf8().data());
                    emit finished(false);

                    return;
                }
            }

            m_localFile = new QFile(m_filePathName);
            if (!m_localFile->open(QFile::WriteOnly))
            {
                QString strCreateFailed = (tr("创建文件 %1 失败！").arg(m_filePathName));
                CLOG::Out("%s", strCreateFailed.toUtf8().data());
                return;
            }

            //注意此处是赋值而不是+=
            m_bytesReceived = m_filePathSize;

            if (m_lbStatus)
            {
                m_timer.restart();
            }
        }
    }
    else  //正式读取文件内容
    {
        qint64 nbytesAvailable = m_tcpReceivedSocket->bytesAvailable();

        if (m_bytesReceived + nbytesAvailable <= m_totalBytes)
        {
            m_inBlock = m_tcpReceivedSocket->readAll();
            m_bytesReceived += m_inBlock.size();
        }
        else
        {
            m_inBlock = m_tcpReceivedSocket->read(m_totalBytes - m_bytesReceived);
            m_bytesReceived += m_inBlock.size();
        }
        m_localFile->write(m_inBlock);
        m_inBlock.clear();

        if (m_lbStatus)
        {
            float useTime = m_timer.elapsed();
            double speed = m_bytesReceived / useTime;

            m_lbStatus->setText(tr("已接收 %1MB (%2MB/s) \n共%3MB 已用时:%4秒\n估计剩余时间：%5秒")
                                .arg(m_bytesReceived / (1024*1024))//已接收
                                .arg(speed*1000/(1024*1024),0,'f',2)//速度
                                .arg(m_totalBytes / (1024 * 1024))//总大小
                                .arg(useTime/1000,0,'f',0)//用时
                                .arg(m_totalBytes/speed/1000 - useTime/1000,0,'f',0));//剩余时间
        }

        emit progress(m_bytesReceived, m_totalBytes);
    }

    if(m_bytesReceived == m_totalBytes)
    {
        m_localFile->flush();
        m_localFile->close();
        m_localFile = nullptr;

        m_inBlock.clear();
        m_bytesReceived = 0;
        m_totalBytes = 0;
        m_filePathSize = 0;

        emit finished(true);
    }
    else if (m_bytesReceived > m_totalBytes)
    {
        CLOG::Out("超量接收！请增加发送延迟！");
    }
}

void TcpUploadServer::displayError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)

    QString strErr = m_tcpReceivedSocket->errorString();
    CLOG::Out("%s", strErr.toUtf8().data());

    m_busy = false;

    if (m_lbStatus)
    {
        m_lbStatus->setText(m_tcpReceivedSocket->errorString());
    }
}
