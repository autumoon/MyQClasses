#include "HttpUpload.h"
#include "Log.h"

HttpUpload::HttpUpload(QObject *parent) : QObject(parent)
{
    Initialize();
    if (parent)
    {
        m_manager = new QNetworkAccessManager(parent);
    }
}

int HttpUpload::uploadDir(const QUrl &url, const QByteArray &baParameter)
{
    QNetworkRequest request(url);

    request.setRawHeader("Content-Type", "charset='utf-8'");
    request.setRawHeader("Content-Type", "application/json");

    if (m_bToken)
    {
        request.setRawHeader(QByteArray("Authorization"),m_strToken.toUtf8());
    }

    QNetworkReply* reply = m_manager->post(request, baParameter);

    //关联信号与槽
    connect(reply, SIGNAL(finished()), this, SIGNAL(finished()));
    connect(reply, SIGNAL(finished()), this, SLOT(SendFinishedReply()));
    connect(reply, SIGNAL(progress(qint64,qint64)), this, SIGNAL(progress(qint64,qint64)));

    if (reply == nullptr)
    {
        CLOG::Out("上传post请求失败!");
        return -1;
    }

    m_reply = reply;

    return 0;
}

void HttpUpload::Initialize()
{
    m_bToken = false;
    m_manager = nullptr;
    m_reply = nullptr;
}

void HttpUpload::SendFinishedReply()
{
    if (m_reply == nullptr)
    {
        return;
    }

    QString strInfo = "";
    // 获取http状态码
    QVariant statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(statusCode.isValid())
        strInfo += "status code = " + statusCode.toString() + '\n';

    QVariant reason = m_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if(reason.isValid())
        strInfo += "reason = " + reason.toString() + '\n';

    QNetworkReply::NetworkError err = m_reply->error();
    if(err != QNetworkReply::NoError)
    {
        strInfo += "Failed: " + m_reply->errorString() + '\n';
    }
    else
    {
        //输出内容
        if (m_reply->error() == QNetworkReply::NoError)
        {
            //假设成功获取内容
            emit finishedWidthReply(m_reply->readAll());
            return;
        }
        else
        {
            CLOG::Out("error:%s",  m_reply->errorString().toUtf8().data());
        }
    }

    m_reply->deleteLater();
    //失败则发送失败原因
    finishedWidthReply(strInfo.toUtf8());
}
