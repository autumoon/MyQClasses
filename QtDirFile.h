#ifndef QTDIRFILE_H
#define QTDIRFILE_H

#include <QObject>
#include <QStringList>

#if defined(Q_OS_LINUX)

#ifndef PATH_SEP
#define PATH_SEP '/'
#endif

#elif defined(Q_OS_WINDOWS)

#ifndef PATH_SEP
#define PATH_SEP '\\'
#endif

#endif

class QTDirFile : public QObject
{
    Q_OBJECT
public:
    explicit QTDirFile(QObject *parent = nullptr);

    static QString BrowseDir(QWidget *parent = nullptr, const QString& strInfo = "请选择目录");

    static QString OpenSuffixFile(QWidget *parent = nullptr, QString strSuffix = ".*");

    //Windows下的反斜杠替换
    inline static QString TransSlashIfNecessary(QString& strLinuxPath);

    static int getDirs(const QString& strDir, QStringList& lPathNames, bool bIncludeSub = true);

    static int getFiles(const QString& strDir, QStringList& lPathNames, QString strSuffix = "*.*", bool bIncludeSub = true);

    static int OpenMultiFiles(QWidget *parent, QString strClassName, QStringList &arrDstFiles, const int nSuffix, ...);							//打开多个文件，每次打开一种类型

signals:

public slots:

private:
    //如果末尾需要加上cSep,注意传入的参数为引用
    static QString AddSlashIfNecessary(QString& strPath, const QChar& cSep = '/') {if (!strPath.isEmpty() && !strPath.endsWith(cSep)) strPath += cSep; return strPath;}
    //如果末尾需要去掉cSep,注意传入的参数为引用
    static QString DelSlashIfNecessary(QString& strPath, const QChar& cSep = '/') {if (!strPath.isEmpty() && strPath.endsWith(cSep)) strPath = strPath.left(strPath.length() - 1); return strPath;}
    //只剩下后缀名，不包含*和.
   static QString LeftSuffixOnly(QString strPathOrFilter, bool bToLower = false);
};

QString QTDirFile::TransSlashIfNecessary(QString &strLinuxPath)
{
#ifdef Q_OS_WINDOWS
        strLinuxPath.replace('/', '\\');
#endif

        return  strLinuxPath;
}

#endif // QTDIRFILE_H
