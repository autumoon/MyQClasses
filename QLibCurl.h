#ifndef QLIBCURL_H
#define QLIBCURL_H

#include <QObject>
#include <QThread>
#include <QMutex>

class QLibCurl : public QObject
{
    Q_OBJECT
public:
    explicit QLibCurl(QObject *parent = nullptr);

    //直接下载
    bool DownLoad(std::string strUrl, std::string strFullPath);
    //断点传输
    bool DownLoad(std::string strUrl, long startPos, long endPos, std::string strFullPath, long seekPos);

Q_SIGNALS:
    void finished(bool);

private:
    bool failed();
    bool succeeded();
    long getDownloadFileLenth (const char *url);

    std::string getDirOfFile(const std::string& strFilePath);
    std::string getNameOfFile(const std::string& strFilePath);

    static int nProgress(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded);
    static size_t writeFunc(void *ptr, size_t size, size_t nmemb, void *userdata);

private:
    FILE *m_fp;
    bool m_busy;
};

class CurlWorker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(const QString& strUrl, const QString& strFullPath)
    {
        /*耗时操作*/
        QLibCurl* lc = new QLibCurl;
        connect(lc, SIGNAL(finished(bool)), this, SIGNAL(finished(bool)));
        lc->DownLoad(strUrl.toUtf8().data(), strFullPath.toUtf8().data());
    }

Q_SIGNALS:
    void finished(bool);
};

class ThreadCurl : public QObject
{
    Q_OBJECT

    QThread workerThread;

public:
    ThreadCurl()
    {
        CurlWorker* worker = new CurlWorker;
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(worker, SIGNAL(finished(bool)), this, SIGNAL(finished(bool)));
        connect(this, SIGNAL(download(const QString&, const QString&)), worker, SLOT(doWork(const QString&, const QString&)));
        workerThread.start();
    }

    ~ThreadCurl()
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
class ThreadCurlList : public QObject
{
    Q_OBJECT
public:
    ThreadCurlList()
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
            ThreadCurl *td = new ThreadCurl;
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
            ThreadCurl *td = new ThreadCurl;
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
