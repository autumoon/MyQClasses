#include "StdDirFile.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>

QString CStdStr::AddQuoteIfContains(const QString &strCmd, const QChar &cChar, bool bAdd)
{
    //去掉两边的引号
    QString strRes(strCmd);

    TrimLeft(strRes, '\"');
    TrimRight(strRes, '\"');

    if (bAdd && strRes.indexOf(cChar) != -1)
    {
        strRes = "\"" + strRes + "\"";
    }

    return strRes;
}

QString CStdStr::AddSlashIfNeeded(const QString &strPath, const QChar &cSep, bool bAdd)
{
    QString strRes(strPath);
    TrimRight(strRes, cSep);

    if (bAdd && !strPath.isEmpty())
    {
        strRes += cSep;
    }

    return  strRes;
}

bool CStdDir::createDirectory(const QString &target, bool bTargetIsFile /*= false*/)
{
    if (target.isEmpty())
        return false;

    QFileInfo dirt(target);
    if (dirt.isDir())
        return true;

    QString strDir(target);

    if (bTargetIsFile)
    {
        int idx = target.lastIndexOf('/');
        strDir = target.mid(0,idx);
    }

    QDir dir;
    return  dir.mkpath(strDir);
}

bool CStdFile::CopyAFile(const QString &strSrcPath, const QString &strDstPath, bool bFailIfExiset)
{
    QFileInfo fiSrc(strSrcPath);

    if (!fiSrc.exists())
    {
        return false;
    }

    QFileInfo fiDst(strDstPath);

    if (fiDst.exists())
    {
        if (strDstPath == strSrcPath || bFailIfExiset)
        {
            return false;
        }
        if (!QFile::remove(strDstPath))
        {
            return false;
        }
    }

    if (!CStdDir::createDirectory(strDstPath, true))
    {
        return false;
    }

    bool bRes = true;

    QFile source(strSrcPath);
    QString dest = strDstPath;
    if (!source.copy(dest))
    {
        bRes = false;
    }

    return bRes;
}

int CStdFile::ParseTxtFile(const QString &stTxtPath, QStringList &lContent)
{
    QFile file(stTxtPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return -1;
    }
    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        lContent.push_back(QString(line));
    }

    return 0;
}

int CStdFile::SaveTXTFile(const QString& stTxtPath, const QStringList& lContent, bool bAppend /*= false*/)
{
    QFile file(stTxtPath);

    if (bAppend)
    {
        if(!file.open(QFile::WriteOnly | QFile::Text | QFile::Append))
        {
            return -1;
        }
    }
    else
    {
        if(!file.open(QFile::WriteOnly | QFile::Text))
        {
            return -1;
        }
    }

    QTextStream filestream(&file);

    int nListCount = lContent.size();

    for (int i = 0; i < nListCount; ++i)
    {
        filestream << lContent[i];
    }

    file.close();

    return 0;
}

int CStdFile::SaveTXTFile(const QStringList& lContent, const QString& stTxtPath, bool bAppend /*= false*/)
{
    QFile file(stTxtPath);

    if (bAppend)
    {
        if(!file.open(QFile::WriteOnly | QFile::Text | QFile::Append))
        {
            return -1;
        }
    }
    else
    {
        if(!file.open(QFile::WriteOnly | QFile::Text))
        {
            return -1;
        }
    }

    QTextStream filestream(&file);

    int nListCount = lContent.size();

    for (int i = 0; i < nListCount; ++i)
    {
        filestream << lContent[i] << "\n";
    }

    file.close();

    return 0;
}

QString CStdStr::GetDirOfDir(QString strDirPath, const QChar &cSep)
{
    if (strDirPath.isEmpty()) return  strDirPath;

    CStdStr::AddSlashIfNeeded(strDirPath, cSep, false);
    CStdStr::AddSlashIfNeeded(strDirPath, ':', false);
    strDirPath = strDirPath.left(strDirPath.lastIndexOf(cSep));

    return  strDirPath;
}

QString CStdStr::GetDirOfFile(QString strFilePath, const QChar &cSep)
{
    if (strFilePath.isEmpty()) return  strFilePath;

    CStdStr::AddSlashIfNeeded(strFilePath, cSep, false);
    CStdStr::AddSlashIfNeeded(strFilePath, ':', false);
    strFilePath = strFilePath.left(strFilePath.lastIndexOf(cSep));

    return  strFilePath;
}

QString CStdStr::GetNameOfDir(QString strDirPath, const QChar &cSep)
{
    if (strDirPath.isEmpty()) return  strDirPath;

    CStdStr::AddSlashIfNeeded(strDirPath, cSep, false);
    CStdStr::AddSlashIfNeeded(strDirPath, ':', false);
    strDirPath = strDirPath.right(strDirPath.length() - strDirPath.lastIndexOf(cSep) - 1);

    return  strDirPath;
}

QString CStdStr::GetNameOfFile(QString strFilePath, const QChar &cSep)
{
    if (strFilePath.isEmpty() || strFilePath.endsWith(cSep)) return  strFilePath;

    strFilePath = strFilePath.right(strFilePath.length() - strFilePath.lastIndexOf(cSep) - 1);

    return  strFilePath;
}

QString CStdStr::LeftSuffixOnly(QString strPathOrFilter, bool bToLower /*= false*/)
{
    if (strPathOrFilter.isEmpty() || strPathOrFilter.endsWith('/')) return  strPathOrFilter;

    //没有后缀
    int nLastPointIndex = strPathOrFilter.lastIndexOf('.');
    if (nLastPointIndex  == -1)
    {
        return "";
    }

    strPathOrFilter = strPathOrFilter.right(strPathOrFilter.length() - nLastPointIndex - 1);

    if (bToLower)
    {
        strPathOrFilter = strPathOrFilter.toLower();
    }

    return strPathOrFilter;
}

QString CStdStr::TrimLeft(QString &strToTrim, const QChar &cToTrim)
{
    while (strToTrim.startsWith(cToTrim))
    {
        strToTrim = strToTrim.right(strToTrim.length() - 1);
    }

    return  strToTrim;
}

QString CStdStr::TrimRight(QString &strToTrim, const QChar &cToTrim)
{
    while (strToTrim.endsWith(cToTrim))
    {
        strToTrim = strToTrim.left(strToTrim.length() - 1);
    }

    return  strToTrim;
}
