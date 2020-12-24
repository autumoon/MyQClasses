#ifndef QSTDDIRFILE_H
#define QSTDDIRFILE_H

#include <QFile>

#if defined(Q_OS_LINUX)

#ifndef PATH_SEP
#define PATH_SEP '/'
#endif

#elif defined(Q_OS_WINDOWS)

#ifndef PATH_SEP
#define PATH_SEP '\\'
#endif

#endif

class CStdStr
{
public:
    //如果包含空格需要添加引号
    static QString AddQuoteIfContains(const QString& strCmd, const QChar& cChar = ' ', bool bAdd = true);

    //如果末尾需要加上cSep
    static QString AddSlashIfNeeded(const QString& strPath, const QChar& cSep = '/',  bool bAdd = true);

    //获取某个文件夹的上一级目录,文件夹不一定需要存在
    static QString GetDirOfDir(QString strDirPath, const QChar& cSep = '/');

    //获取某个文件夹的上一级目录,文件夹不一定需要存在
    static QString GetDirOfFile(QString strFilePath, const QChar& cSep = '/');

    //获取某个文件夹的名字,文件夹不一定需要存在
    static QString GetNameOfDir(QString strDirPath, const QChar& cSep = '/');

    //获取某个文件的名字,文件不一定需要存在
    static QString GetNameOfFile(QString strFilePath, const QChar& cSep = '/');

    //只保留后缀
    static QString LeftSuffixOnly(QString strPathOrFilter, bool bToLower = false);

    //清除左边的某个符号,一直清除到左边不存在该符号为止
    static QString TrimLeft(QString& strToTrim, const QChar& cToTrim = '/');

    //清除右边的某个符号,一直清除到右边不存在该符号为止
    static QString TrimRight(QString& strToTrim, const QChar& cToTrim = '/');
};

class CStdDir
{
public:
    //内部已经判断文件夹是否已经存在
    static bool createDirectory(const QString& target, bool bTargetIsFile = false);
};

class CStdFile
{
public:
    static bool CopyAFile(const QString& strSrcPath, const QString& strDstPath, bool bFailIfExiset = false);

    static int ParseTxtFile(const QString& stTxtPath, QStringList& lContent);

    //lContent在前时自动添加换行符
    static int SaveTXTFile(const QString& stTxtPath, const QStringList& lContent, bool bAppend = false);
    static int SaveTXTFile(const QStringList& lContent, const QString& stTxtPath, bool bAppend = false);

};


#endif
