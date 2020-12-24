#include "QLibCurl.h"
#include "curl/curl.h"
#include <QEventLoop>
#include <QTimer>
#include <QDir>

struct tNode
{
    FILE *fp;
    long startPos;
    long endPos;
    void *curl;
};

QLibCurl::QLibCurl(QObject *parent) : QObject(parent)
{
    m_fp = nullptr;
    m_busy = false;
}

bool QLibCurl::DownLoad(std::string strUrl, std::string strFullPath)
{
    std::string strDir = getDirOfFile(strFullPath);
    std::string strFile = getNameOfFile(strFullPath);

    if(m_busy)
    {
        return failed();
    }
    else
    {
        m_busy = true;
    }

    long fileLength = getDownloadFileLenth (strUrl.c_str ());

    if (fileLength <= 0)
    {
        printf ("get the file length error...");
        return failed();
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
    return DownLoad(strUrl, 0, fileLength, strFullPath, 0);
}

bool QLibCurl::DownLoad(std::string strUrl, long startPos, long endPos, std::string strFullPath, long seekPos)
{
    const std::string outFileName = strFullPath;
    FILE *fp = fopen (outFileName.c_str (), "wb");
    if (!fp)
    {
        return failed();
    }

    CURL *curl = curl_easy_init();

    tNode *pNode = new tNode;
    pNode->fp = fp;
    pNode->startPos = startPos;
    pNode->endPos = endPos;
    pNode->curl = curl;

    char range[64] = { 0 };
    snprintf (range, sizeof (range), "%ld-%ld", startPos, endPos);

    // Download pacakge
    curl_easy_setopt (curl, CURLOPT_URL, strUrl.c_str ());
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writeFunc);
    curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *) pNode);
    curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 0L);

    curl_easy_setopt (curl, CURLOPT_PROGRESSFUNCTION, nProgress);

    curl_easy_setopt (curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt (curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
    curl_easy_setopt (curl, CURLOPT_LOW_SPEED_TIME, 5L);
    curl_easy_setopt (curl, CURLOPT_RANGE, range);

    int res = curl_easy_perform (curl);

    if (res != 0)
    {
        return failed();
    }


    fclose (fp);

    //printf ("%s download succed......\n", strUrl.c_str());

    return succeeded();
}

size_t QLibCurl::writeFunc(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    tNode *node = (tNode *) userdata;
    size_t written = 0;
    if (node->startPos + size * nmemb <= node->endPos)
    {
        fseek (node->fp, node->startPos, SEEK_SET);
        written = fwrite (ptr, size, nmemb, node->fp);
        node->startPos += size * nmemb;
    }
    else
    {
        fseek (node->fp, node->startPos, SEEK_SET);
        written = fwrite (ptr, 1, node->endPos - node->startPos + 1, node->fp);
        node->startPos = node->endPos;
    }

    return written;
}

bool QLibCurl::failed()
{
    m_fp = nullptr;
    m_busy = false;
    emit finished(false);

    return false;
}

bool QLibCurl::succeeded()
{
    m_fp = nullptr;
    m_busy = false;
    emit finished(true);

    return true;
}

long QLibCurl::getDownloadFileLenth(const char *url)
{
    double downloadFileLenth = 0;
    CURL *handle = curl_easy_init ();
    curl_easy_setopt (handle, CURLOPT_URL, url);
    curl_easy_setopt (handle, CURLOPT_HEADER, 1);	//只需要header头
    curl_easy_setopt (handle, CURLOPT_NOBODY, 1);	//不需要body
    if (curl_easy_perform (handle) == CURLE_OK)
    {
        curl_easy_getinfo (handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLenth);
    }
    else
    {
        downloadFileLenth = -1;
    }
    return downloadFileLenth;
}

std::string QLibCurl::getDirOfFile(const std::string &strFile)
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

std::string QLibCurl::getNameOfFile(const std::string &strFilePath)
{
    int index = (int)strFilePath.rfind('/');
    std::string strFileName = strFilePath.substr(index + 1, strFilePath.length() - index - 1);

    return strFileName;
}

int QLibCurl::nProgress(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
    Q_UNUSED(ptr)
    Q_UNUSED(nowUpLoaded)
    Q_UNUSED(totalToUpLoad)

    //printf("%lf/%lf", nowDownloaded, totalToDownload);

    return 0;
}
