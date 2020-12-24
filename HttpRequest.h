#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>
#include <QEventLoop>

//请求方式，之所以使用宏定义是因为方便信号传输
#define     HTTP_POST         1
#define     HTTP_GET            2
#define     HTTP_PUT            3
#define     HTTP_DELETE       4

class HttpRequest : public QObject
{
    Q_OBJECT
public:
    explicit HttpRequest(QObject *parent = nullptr);

    bool RequestHttpUrl(const QString& strUrl, int eHm = HTTP_POST, const QByteArray& baData = QByteArray(),  int msec = 2000);

    //判断主机是否在线
    static bool IsHostOnline(QString strHostName, int nTimeoutmSeconds = 2000);
    //判断是否通外网，能连接百度IP说明可以通外网
    static bool IsWebOk(){return IsHostOnline("202.108.22.5", 2000);}

Q_SIGNALS:
    void reply_data(bool bSuccess, const QByteArray& ba);

private:
    static bool parseReplyData(QNetworkReply* reply, QByteArray& data);

    static void setRequestHeader(QNetworkRequest &request)
    {
        request.setRawHeader("Content-Type", "charset='utf-8'");
        request.setRawHeader("Content-Type", "application/json");
    }
    void sleep(int msec = 5000);
};

#endif // HTTPREQUEST_H

//启用线程调用下载
class requestWorker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(const QString& strUrl, int eHm, const QByteArray& baData, int msec = 2000)
    {
        /* ... here is the expensive or blocking operation ... */
        HttpRequest *hr = new HttpRequest(this);
        connect(hr, SIGNAL(reply_data(bool, const QByteArray&)), this, SIGNAL(reply_data(bool, const QByteArray&)));
        hr->RequestHttpUrl(strUrl, eHm, baData, msec);
    }

Q_SIGNALS:
    void reply_data(bool bSuccess, const QByteArray& ba);
};

class ThreadHttpRequest : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    ThreadHttpRequest()
    {
        requestWorker *worker = new requestWorker;
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &ThreadHttpRequest::request_data, worker, &requestWorker::doWork);
        connect(worker, &requestWorker::reply_data, this, &ThreadHttpRequest::get_result);
        connect(worker, &requestWorker::reply_data, this, &ThreadHttpRequest::reply_data);
        workerThread.start();
    }
    ~ThreadHttpRequest()
    {
        workerThread.quit();
        workerThread.wait();
    }

    //异步，信号返回结果
    void RequestHttpUrl(const QString& strUrl, int eHm, const QByteArray& baData, int msec = 2000)
    {
        emit request_data(strUrl, eHm, baData, msec);
    }

    //同步，直接返回结果
    bool RequestHttpUrl(const QString& strUrl, int eHm, const QByteArray& baData, QByteArray& baRes, int msec = 2000)
    {
        QEventLoop el;
        connect(this, SIGNAL(reply_data(bool, const QByteArray&)), &el, SLOT(quit()));

        emit request_data(strUrl, eHm, baData, msec);

        el.exec(QEventLoop::ExcludeUserInputEvents);

        //获取结果
        baRes = m_baRes;

        return m_bRes;
    }

Q_SIGNALS:
    void reply_data(bool bSuccess, const QByteArray& ba);
    void request_data(const QString& strUrl, int eHm, const QByteArray& baData, int msec);

private slots:
    void get_result(bool bSuccess, const QByteArray& ba)
    {
        m_bRes = bSuccess;
        m_baRes = ba;
    }

private:
    bool m_bRes;
    QByteArray m_baRes;
};

