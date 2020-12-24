#include "HttpRequest.h"
#include <QEventLoop>
#include <QTimer>
#include "JsonParser.h"

HttpRequest::HttpRequest(QObject *parent) : QObject(parent)
{

}

bool HttpRequest::RequestHttpUrl(const QString &strUrl, int eHm, const QByteArray &baData, int msec)
{
    QNetworkRequest request(strUrl);
    setRequestHeader(request);

    QNetworkAccessManager* naManager = new QNetworkAccessManager(this);

    QEventLoop eventloop;
    QTimer timer;
    timer.singleShot(msec, &eventloop, SLOT(quit()));
    timer.start();

    QNetworkReply* reply = nullptr;

    switch (eHm)
    {
    case HTTP_POST:
        reply = naManager->post(request, baData);
        break;
    case HTTP_PUT:
        reply = naManager->put(request, baData);
        break;
    case HTTP_DELETE:
        reply = naManager->deleteResource(request);
        break;

    case HTTP_GET:
    default:
        reply = naManager->get(request);
        break;
    }

    QMetaObject::Connection conRet = QObject::connect(reply, SIGNAL(finished()), &eventloop, SLOT(quit()));
    Q_ASSERT(conRet);

    eventloop.exec(QEventLoop::ExcludeUserInputEvents);

    QByteArray baReply;
    if (timer.isActive() && parseReplyData(reply, baReply))
    {
        reply->deleteLater();
        emit reply_data(true, baReply);

        return true;
    }

    //超时，未知状态
    disconnect(reply, SIGNAL(finished()), &eventloop, SLOT(quit()));
    reply->abort();
    reply->deleteLater();
    emit reply_data(false, baReply);

    return false;
}

bool HttpRequest::IsHostOnline(QString strHostName, int nTimeoutmSeconds)
{
    QNetworkRequest request(strHostName);
    setRequestHeader(request);
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

bool HttpRequest::parseReplyData(QNetworkReply *reply, QByteArray &data)
{
    if (reply == nullptr)
    {
        data = QString("Unbelievable! The reply pointer is nullptr!").toUtf8();

        return false;
    }

    QString strResult;

    // 获取http状态码
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(statusCode.isValid())
        strResult += "status code=" + statusCode.toString() + '\n';

    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if(reason.isValid())
        strResult += "reason=" + reason.toString() + '\n';

    QNetworkReply::NetworkError err = reply->error();

    if(err != QNetworkReply::NoError)
    {
        strResult += "The err code is " + QString::number(err) + ";";
        data = strResult.toUtf8();

        return false;
    }
    else
    {
        //输出内容
        data = reply->readAll();
    }

    return true;
}

void HttpRequest::sleep(int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}
