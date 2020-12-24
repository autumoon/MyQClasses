#ifndef HTTPUPLOAD_H
#define HTTPUPLOAD_H

#include <QObject>
#include <QThread>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

class HttpUpload : public QObject
{
    Q_OBJECT
public:
    explicit HttpUpload(QObject *parent = nullptr);

    //上传某个目录
    int uploadDir(const QUrl& url, const QByteArray& baParameter);

    void Initialize();
    void SetToken(const QString& strToken){if (!strToken.isEmpty()){ m_bToken = true; m_strToken = strToken;}}

signals:
    void finished();
    void finishedWidthReply(const QByteArray& replyData);
    void progress(qint64 bytesSent, qint64 bytesTotal);
public slots:
    void SendFinishedReply();
private:
    //上传公共指针
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_reply;
    QUrl m_url;

    //token
    bool m_bToken;
    QString m_strToken;
};

//启用线程调用下载
class uploadWorker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(const QUrl& url, const QByteArray& baParameter, const QString& token = "")
    {
        /* ... here is the expensive or blocking operation ... */
        HttpUpload *up = new HttpUpload(this);
        connect(up, SIGNAL(progress(qint64, qint64)), this, SIGNAL(progress(qint64, qint64)));
        connect(up, SIGNAL(finished()), this, SIGNAL(finished()));
        connect(up, SIGNAL(finishedWidthReply(const QByteArray&)), this, SIGNAL(finishedWidthReply(const QByteArray&)));

        if (!token.isEmpty())
        {
            up->SetToken(token);
        }

        int res = up->uploadDir(url, baParameter);
        //注意这个是异步的，只能表示成功开始下载
        emit resultReady(res);
    }

signals:
    void resultReady(const int& result);
    void progress(qint64, qint64);
    void finished();
    void finishedWidthReply(const QByteArray& replyData);
};

class Threadupload : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    Threadupload()
    {
        uploadWorker *worker = new uploadWorker;
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &Threadupload::operate, worker, &uploadWorker::doWork);
        connect(worker, &uploadWorker::resultReady, this, &Threadupload::resultReady);
        connect(worker, SIGNAL(progress(qint64, qint64)), this, SIGNAL(progress(qint64, qint64)));
        connect(worker, SIGNAL(finished()), this, SIGNAL(finished()));
        connect(worker, SIGNAL(finishedWidthReply(const QByteArray&)), this, SIGNAL(finishedWidthReply(const QByteArray&)));
        workerThread.start();
    }
    ~Threadupload()
    {
        workerThread.quit();
        workerThread.wait();
    }

    void upload(const QString& url, const QByteArray& baParameter, const QString& token = "")
    {
        emit operate(url, baParameter, token);
    }

    void upload(const QUrl& url, const QByteArray& baParameter, const QString& token = "")
    {
        emit operate(url, baParameter, token);
    }

public slots:

signals:
    void operate(const QUrl& url, const QByteArray& baParameter, const QString& token = "");
    void resultReady(const int& result);
    void progress(qint64, qint64);
    void finished();
    void finishedWidthReply(const QByteArray& replyData);
};

#endif // HTTPUPLOAD_H
