#ifndef QLIBCURL_H
#define QLIBCURL_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QFile>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

class HttpDownload : public QObject
{
    Q_OBJECT
public:
    explicit HttpDownload(QObject *parent = nullptr);

    //直接下载
    bool DownLoad(std::string strUrl, std::string strFullPath);

    //判断主机是否在线
    static bool IsHostOnline(QString strHostName, int nTimeoutmSeconds = 2000);
    //判断是否通外网，能连接百度IP说明可以通外网
    static bool IsWebOk(){return IsHostOnline("202.108.22.5", 2000);}

Q_SIGNALS:
    void readyRead();
    void progress(qint64, qint64);
    void finished(bool);

private:
    bool failed();
    bool succeeded();

    QString getFileName(QString strUrl);

    std::string getDirOfFile(const std::string& strFilePath);
    std::string getNameOfFile(const std::string& strFilePath);

    void Initialize();
    void SetToken(const QString& strToken){if (!strToken.isEmpty()){ m_bToken = true; m_strToken = strToken;}}

    static void deleteFile(QFile *myfile);

private slots:
    void httpReadReady();
    void httpRequestFinish();

private:
    //下载公共指针
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_reply;
    QFile* m_file;
    QUrl m_url;

    //token
    QString m_strFileName;
    QString m_strToken;
    bool m_bToken;
    bool m_busy;
};

class HttpDownWorker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(const QString& strUrl, const QString& strFullPath)
    {
        /*耗时操作*/
        HttpDownload* hd = new HttpDownload;
        connect(hd, SIGNAL(finished(bool)), this, SIGNAL(finished(bool)), Qt::DirectConnection);
        hd->DownLoad(strUrl.toUtf8().data(), strFullPath.toUtf8().data());
    }

Q_SIGNALS:
    void finished(bool);
};

class ThreadHttpDown : public QObject
{
    Q_OBJECT

    QThread workerThread;

public:
    ThreadHttpDown()
    {
        HttpDownWorker* worker = new HttpDownWorker;
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(worker, SIGNAL(finished(bool)), this, SIGNAL(finished(bool)), Qt::DirectConnection);
        connect(this, SIGNAL(download(const QString&, const QString&)), worker, SLOT(doWork(const QString&, const QString&)));
        workerThread.start();
    }

    ~ThreadHttpDown()
    {
        workerThread.quit();
        workerThread.wait();
    }

    bool DownLoad(std::string strUrl, std::string strFullPath)
    {
        emit download(strUrl.c_str(), strFullPath.c_str());
        return true;
    }

Q_SIGNALS:
    void finished(bool);
    void download(const QString& strUrl, const QString& strFullPath);
};

//使用固定的线程数目下载多个地址
class ThreadHttpDownList : public QObject
{
    Q_OBJECT
public:
    ThreadHttpDownList()
    {
        m_nThreadCount = 4;
        m_nTaskCount = 0;
        m_nFinishedCount = 0;
    }

    void StartDownload(const QStringList& lFilePaths, const QStringList& lDstFilePaths, const int& nThreadNum = 4)
    {
        m_nThreadCount = nThreadNum;

        m_lSrcFiles = lFilePaths;
        m_lDstFiles = lDstFilePaths;

        m_nAllTaskCount = lFilePaths.size();
        int nTaskCount = qMin(m_nThreadCount, m_nAllTaskCount);
        for (int i = 0; i < nTaskCount; ++i)
        {
            ThreadHttpDown *td = new ThreadHttpDown;
            connect(td, SIGNAL(finished(bool)), this, SLOT(UpdateTaskCount()), Qt::DirectConnection);
            m_mutex.lock();
            ++m_nTaskCount;
            //内部单线程
            td->DownLoad(m_lSrcFiles.at(0).toUtf8().data(), m_lDstFiles.at(0).toUtf8().data());
            m_lSrcFiles.pop_front();
            m_lDstFiles.pop_front();
            m_mutex.unlock();
        }
    }

public slots:
    void UpdateTaskCount()
    {
        m_mutex.lock();
        --m_nTaskCount;
        ++m_nFinishedCount;
        emit progress(m_nFinishedCount, m_nAllTaskCount);

        if (m_nFinishedCount < m_nAllTaskCount && m_lSrcFiles.size())
        {
            ThreadHttpDown *td = new ThreadHttpDown;
            connect(td, SIGNAL(finished(bool)), this, SLOT(UpdateTaskCount()), Qt::DirectConnection);
            ++m_nTaskCount;
            //内部单线程
            td->DownLoad(m_lSrcFiles.at(0).toUtf8().data(), m_lDstFiles.at(0).toUtf8().data());
            m_lSrcFiles.pop_front();
            m_lDstFiles.pop_front();
        }
        else if (m_nTaskCount == 0)
        {
            emit finished();
        }

        m_mutex.unlock();
    }

signals:
    void progress(qint64, qint64);
    void finished();

private:
    int m_nThreadCount;
    int m_nTaskCount;
    int m_nFinishedCount;
    int m_nAllTaskCount;
    QMutex m_mutex;
    QStringList m_lSrcFiles;
    QStringList m_lDstFiles;
};

#endif // QLIBCURL_H
