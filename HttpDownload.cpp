#include "HttpDownload.h"
#include <QEventLoop>
#include <QTimer>
#include <QDir>

HttpDownload::HttpDownload(QObject *parent) : QObject(parent)
{
    Initialize();
    if (parent)
    {
        m_manager = new QNetworkAccessManager(parent);
    }
    else
    {
        m_manager = new QNetworkAccessManager(this);
    }
}

bool HttpDownload::DownLoad(std::string strUrl, std::string strFullPath)
{
    std::string strDir = getDirOfFile(strFullPath);
    std::string strFile = getNameOfFile(strFullPath);

    m_strFileName = strFile.c_str();

    if(m_busy)
    {
        return failed();
    }
    else
    {
        m_busy = true;
    }

    // Create a file to save package.
    if (strDir.length() > 1 && strDir.at(strDir.length() - 1) != '/')
    {
        strDir += '/';
    }

    //创建目标目录
    QDir dir;
    if (!dir.mkpath(strDir.c_str()))
    {
        return failed();
    }

    //qDebug() << strUrl.c_str() << "->" << strFullPath.c_str();

    //开始下载
    if (strFile.length() == 0)
    {
        m_strFileName = getFileName(strUrl.c_str());
        if(m_strFileName.isEmpty())
        {
            m_strFileName = "index.html";
        }
    }

    //将文件下载到临时目录中(在文件名的前面添加临时目录的名称)
    QString strfileName = m_strFileName;
    strfileName = QString(strDir.c_str()) + strfileName;
    m_file = new QFile();
    m_file->setFileName(strfileName);
    if(!m_file->open(QIODevice::WriteOnly))
    {
        deleteFile(m_file);
        return -1;
    }

    //QByteArray path = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");

    QNetworkRequest request(QString(strUrl.c_str()));
    request.setRawHeader("Content-Type", "charset='utf-8'");
    request.setRawHeader("Content-Type", "application/json");

    if (m_bToken)
    {
        request.setRawHeader(QByteArray("Authorization"),m_strToken.toUtf8());
    }

    QNetworkReply* reply = m_manager->get(request);

    //关联信号与槽
    connect(reply, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
    connect(reply, SIGNAL(readyRead()), this, SLOT(httpReadReady()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(progress(qint64,qint64)));
    connect(reply, SIGNAL(finished()), this, SLOT(httpRequestFinish()));

    m_reply = reply;

    return true;
}

bool HttpDownload::IsHostOnline(QString strHostName, int nTimeoutmSeconds)
{
    QNetworkRequest request(strHostName);
    request.setRawHeader("Content-Type", "charset='utf-8'");
    request.setRawHeader("Content-Type", "application/json");
    QNetworkAccessManager* naManager = new QNetworkAccessManager;

    QEventLoop eventloop;

    QTimer timer;
    timer.singleShot(nTimeoutmSeconds, &eventloop, SLOT(quit()));
    timer.start();

    QNetworkReply* reply = naManager->get(request);
    QMetaObject::Connection conRet = QObject::connect(reply, SIGNAL(finished()), &eventloop, SLOT(quit()));
    Q_ASSERT(conRet);

    eventloop.exec(QEventLoop::ExcludeUserInputEvents);

    if (!timer.isActive())
    {
        //超时，未知状态
        disconnect(reply, SIGNAL(finished()), &eventloop, SLOT(quit()));
        reply->abort();
        reply->deleteLater();

        return false;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        reply->abort();
        reply->deleteLater();
        return false;
    }

    bool bRes = reply->readAll().length() > 0;
    reply->abort();
    reply->deleteLater();

    return bRes;
}

bool HttpDownload::failed()
{
    m_busy = false;
    emit finished(false);

    return false;
}

bool HttpDownload::succeeded()
{
    m_busy = false;
    emit finished(true);

    return true;
}

QString HttpDownload::getFileName(QString strUrl)
{
    QFileInfo fileInfo (strUrl);
    QString strFileName = fileInfo.fileName();

    return strFileName;
}

std::string HttpDownload::getDirOfFile(const std::string &strFile)
{
    std::string strFilePath(strFile);

    size_t index = strFilePath.rfind('/');

    if (index != std::string::npos)
    {
        return strFilePath.substr(0, index);
    }
    else
    {
        return strFilePath;
    }
}

std::string HttpDownload::getNameOfFile(const std::string &strFilePath)
{
    int index = (int)strFilePath.rfind('/');
    std::string strFileName = strFilePath.substr(index + 1, strFilePath.length() - index - 1);

    return strFileName;
}

void HttpDownload::Initialize()
{
    //默认全部初始化
    m_manager = nullptr;
    m_reply = nullptr;
    m_file = nullptr;
    m_busy = false;
    m_bToken = false;
}

void HttpDownload::deleteFile(QFile *myfile)
{
    myfile->close();
    delete myfile;
    myfile = nullptr;
}

void HttpDownload::httpReadReady()
{
    if (m_file) m_file->write(m_reply->readAll());  //如果文件存在，则写入文件
}

void HttpDownload::httpRequestFinish()
{
    deleteFile(m_file);
    m_reply->deleteLater();
    m_reply = nullptr;

    succeeded();
}
