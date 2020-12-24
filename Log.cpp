#include "Log.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

//得到可执行程序所在目录
//BOOL bIncludeSep -- 是否包含最后的分隔符
QString GetCorePath(bool bIncludeSep)
{
    QString strCorePath = QCoreApplication::applicationDirPath();

    if (bIncludeSep && !strCorePath.endsWith('/'))
    {
        strCorePath += '/';
    }

    return  strCorePath;
}

//-获取最后的文件名 如果给定文件不是全路径，就是相对于可执行文件
QString GetCoreFilePath(QString strCurFile)
{
    QString strPath = strCurFile;
    if(!strPath.isEmpty())
    {
        //-相对路径-
        if(!strCurFile.startsWith('/'))
        {
            strPath = GetCorePath(true) + strCurFile;
        }
    }
    return strPath;
}

QString CLOG::s_strLogFile = "";
QString CLOG::s_strLogPrefix = "";
QMutex CLOG::s_mutex;

CLOG::CLOG(QObject *parent) : QObject(parent)
{

}

QString CLOG::GetLogFile()
{
    return s_strLogFile;
}

short CLOG::SetLogFile(QString strLogFileName)
{
    if(strLogFileName.isEmpty())
    {
        QDateTime current_date_time = QDateTime::currentDateTime();
        QString current_date = current_date_time.toString("yyyy年MM月dd日");
        s_strLogFile = GetCoreFilePath(current_date + ".log");
    }
    else
        s_strLogFile = GetCoreFilePath(strLogFileName);

    return 1;
}

short CLOG::ViewLogFile()
{
    QDesktopServices::openUrl(QUrl("file:" + s_strLogFile, QUrl::TolerantMode));

    return 0;
}

short CLOG::SetPrefix(QString strPrefix)
{
    if(!strPrefix.isEmpty())
    {
        s_strLogPrefix = strPrefix;
    }
    return 1;
}

QString CLOG::sOutV(QString strType, const char *strFormat, va_list valist)
{
    QString   strPart_Prefix;
    if(!s_strLogPrefix.isEmpty())
    {
        strPart_Prefix = s_strLogPrefix;
    }
    QString   strPart_Time;
    {
        QDateTime dateTime(QDateTime::currentDateTime());
        strPart_Time = dateTime.toString("MM-dd hh:mm:ss zzz");
    }
    QString   strPart_Type;
    if(!strType.isEmpty())
    {
        strPart_Type = strType;
    }
    QString   strPart_Info;
    {
        strPart_Info.vsprintf(strFormat, valist);
    }
    QString str = "[" + strPart_Prefix + strPart_Time + "]" + strPart_Type + strPart_Info;

    return str;
}

QString CLOG::sOut0(QString strType, const char* strFormat /*= nullptr*/,...)
{
    va_list  valist;
    va_start(valist, strFormat);
    QString strInfo = sOutV(strType, strFormat, valist);
    va_end(valist);
    return strInfo;
}

QString CLOG::sOut (const char* strFormat /*= nullptr*/,...)
{
    va_list  valist;
    va_start(valist, strFormat);
    QString strInfo = sOutV("", strFormat, valist);
    va_end(valist);
    return strInfo;
}

short CLOG::Clean()
{
    //-打开关闭文件-
    s_mutex.lock();
    if (s_strLogFile.isEmpty())
        SetLogFile();
    QFile file(s_strLogFile);

    if(file.open(QFile::WriteOnly | QFile::Text))
    {
        setlocale(LC_CTYPE, "chs");//设定f
        file.close();
        setlocale(LC_ALL, "C"); //还原区域设定
    }
    s_mutex.unlock();

    return 1;
}

short CLOG::End(const char *pInfo)
{
    s_mutex.lock();
    //-打开关闭文件-
    if (s_strLogFile.isEmpty())
        SetLogFile();
    QFile file(s_strLogFile);

    if(file.open(QFile::WriteOnly | QFile::Text | QFile::Append))
    {
        setlocale(LC_CTYPE, "chs");//设定f
        QString str(pInfo);
        file.write(pInfo);
        file.close();
        setlocale(LC_ALL, "C"); //还原区域设定
    }
    s_mutex.unlock();

    return 1;
}

short CLOG::OutV(QString strType, const char *strFormat, va_list valist)
{
    s_mutex.lock();
    //-打开关闭文件-
    if(s_strLogFile.isEmpty())
        SetLogFile();
    QFile file(s_strLogFile);
    if(file.open(QFile::WriteOnly | QFile::Text | QFile::Append))
    {
        setlocale(LC_CTYPE, "chs");//设定f
        QString  strPart_NewLine = "\n";
        QString  strInfo = sOutV(strType, strFormat, valist);
        QString  str = strInfo + strPart_NewLine;
        file.write(str.toUtf8());
        file.close();
        setlocale(LC_ALL, "C"); //还原区域设定
    }
    s_mutex.unlock();
    return 1;
}

short CLOG::Out0(QString strType, const char* strFormat /*= nullptr*/,...)
{
    va_list  valist;
    va_start(valist, strFormat);
    short rtn = OutV(strType, strFormat, valist);
    va_end(valist);
    return rtn;
}

short CLOG::Out(const QString &strInfo)
{
    s_mutex.lock();
    //-打开关闭文件-
    if (s_strLogFile.isEmpty())
        SetLogFile();
    QFile file(s_strLogFile);
    if (file.open(QFile::WriteOnly | QFile::Text | QFile::Append))
    {
        setlocale(LC_CTYPE, "chs");//设定f
        QString  strPart_NewLine = "\n";
        QString   strPart_Prefix;
        if (!s_strLogPrefix.isEmpty())
        {
            strPart_Prefix = s_strLogPrefix;
        }
        QString   strPart_Time;
        {
            QDateTime dateTime(QDateTime::currentDateTime());
            strPart_Time = dateTime.toString("MM-dd hh:mm:ss zzz");
        }

        QString str = "[" + strPart_Prefix + strPart_Time + "]" + strInfo + strPart_NewLine;
        file.write(str.toUtf8());
        file.close();
        setlocale(LC_ALL, "C"); //还原区域设定
    }
    s_mutex.unlock();

    return 1;
}

short CLOG::Out(const char* strFormat /*= nullptr*/, ...)
{
    va_list  valist;
    va_start(valist, strFormat);
    short rtn = OutV("", strFormat, valist);
    va_end(valist);

    return rtn;
}
