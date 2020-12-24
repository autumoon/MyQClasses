//author：autumoon
//联系QQ：4589968
//日期：2020-10-20
#ifndef THREADPROCESSFILE_H
#define THREADPROCESSFILE_H

#include <QObject>
#include <QThread>
#include <QMutex>

//标准的文件处理函数
class ProcessFile : public QObject
{
    Q_OBJECT
public:
    explicit ProcessFile(QObject *parent = nullptr);

signals:
    void finished(bool bSuccess);
public slots:
    bool CopyAFile(const QString &strSrcPath, const QString &strDstPath, bool bFailIfExiset = false);
private:
    bool copyFileSystem(const QString& strSrcPath, const QString& strDstPath);
    bool createDirectory(const QString &target, bool bTargetIsFile = false);
};

//在线程中处理文件
class ThreadProcessFile : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    ThreadProcessFile()
    {
        ProcessFile *worker = new ProcessFile;
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &ThreadProcessFile::copyFile, worker, &ProcessFile::CopyAFile);
        connect(worker, SIGNAL(finished(bool)), this, SIGNAL(finished(bool)));
        workerThread.start();
    }
    ~ThreadProcessFile()
    {
        workerThread.quit();
        workerThread.wait();
    }

    void CopyAFile(const QString &strSrcPath, const QString &strDstPath, bool bFailIfExiset = false)
    {
        //仅仅发送信号
        emit copyFile(strSrcPath, strDstPath, bFailIfExiset);
    }


public slots:

signals:
    void copyFile(const QString &strSrcPath, const QString &strDstPath, bool bFailIfExiset);
    void finished(bool bSuccess);
};

//使用固定的线程数目下载多个地址
class ThreadProcessList : public QObject
{
    Q_OBJECT
public:
    ThreadProcessList()
    {
        //默认单线程
        m_nThreadCount = 1;
        m_nTaskCount = 0;
        m_nFinishedCount = 0;
        m_bFailIfExist = false;
    }

    void SetThreadNum(const int& nThreadNum){m_nThreadCount = nThreadNum;}
    void StartCopyFiles(const QStringList& lSrcFilePaths, const QStringList& lDstFilePaths, bool bFailIfExist = false)
    {
        m_lSrcFiles = lSrcFilePaths;
        m_lDstFiles = lDstFilePaths;
        m_bFailIfExist = bFailIfExist;

        m_nAllTaskCount = qMin(lSrcFilePaths.size(), lDstFilePaths.size());
        int nTaskCount = qMin(m_nThreadCount, m_nAllTaskCount);
        for (int i = 0; i < nTaskCount; ++i)
        {
            ThreadProcessFile *tp = new ThreadProcessFile;
            connect(tp, SIGNAL(finished(bool)), this, SLOT(UpdateTaskCount(bool)));
            m_mutex.lock();
            ++m_nTaskCount;
            tp->CopyAFile(m_lSrcFiles.at(0), m_lDstFiles.at(0), bFailIfExist);
            m_lSrcFiles.pop_front();
            m_lDstFiles.pop_front();
            m_mutex.unlock();
        }
    }

public slots:
    void UpdateTaskCount(bool bSuccess)
    {
        m_mutex.lock();
        --m_nTaskCount;

        if (bSuccess)
        {
            //只有成功才记入
            ++m_nFinishedCount;
        }

        emit ProcessProgress(m_nFinishedCount, m_nAllTaskCount);

        if (m_nFinishedCount < m_nAllTaskCount && m_lSrcFiles.size())
        {
            ThreadProcessFile *tp = new ThreadProcessFile;
            connect(tp, SIGNAL(finished(bool)), this, SLOT(UpdateTaskCount(bool)));
            ++m_nTaskCount;
            tp->CopyAFile(m_lSrcFiles.at(0), m_lDstFiles.at(0), m_bFailIfExist);
            m_lSrcFiles.pop_front();
            m_lDstFiles.pop_front();
        }
        else if (m_nTaskCount == 0)
        {
            emit finished(m_nFinishedCount == m_nAllTaskCount);
        }

        m_mutex.unlock();
    }

signals:
    void ProcessProgress(qint64 nFinishedCount, qint64 nAllTaskCount);
    void finished(bool bSuccess);

private:
    int m_nThreadCount;
    int m_nTaskCount;
    int m_nFinishedCount;
    int m_nAllTaskCount;
    bool m_bFailIfExist;
    QMutex m_mutex;
    QStringList m_lSrcFiles;
    QStringList m_lDstFiles;
};

#endif // THREADPROCESSFILE_H
