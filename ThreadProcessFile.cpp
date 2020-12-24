//author：autumoon
//联系QQ：4589968
//日期：2020-10-20
#include "ThreadProcessFile.h"
#include <QDir>
#include <QFileInfo>
#include "Log.h"

#if defined(Q_OS_LINUX)

#ifndef PATH_SEP
#define PATH_SEP '/'
#endif

#elif defined(Q_OS_WINDOWS)

#ifndef PATH_SEP
#define PATH_SEP '\\'
#endif

#endif

ProcessFile::ProcessFile(QObject *parent) : QObject(parent)
{

}

bool ProcessFile::createDirectory(const QString &target, bool bTargetIsFile /*= false*/)
{
    if (target.isEmpty())
        return false;

    QFileInfo dirt(target);
    if (dirt.isDir())
        return true;

    QString strDir(target);

    if (bTargetIsFile)
    {
        int idx = target.lastIndexOf("/");
        strDir = target.mid(0,idx);
    }

    QDir dir;
    return  dir.mkpath(strDir);
}

bool ProcessFile::CopyAFile(const QString &strSrcPath, const QString &strDstPath, bool bFailIfExiset /*= false*/)
{
    QFileInfo fiSrc(strSrcPath);

    if (!fiSrc.exists())
    {
        CLOG::Out("文件 %s 不存在!", strSrcPath.toUtf8().data());
        emit finished(false);
        return false;
    }

    QFileInfo fiDst(strDstPath);

    if (fiDst.exists())
    {
        if (strDstPath == strSrcPath || bFailIfExiset)
        {
            CLOG::Out("文件 %s 复制失败！源地址和目标地址相同!", strDstPath.toUtf8().data());
            emit finished(false);
            return false;
        }
        if (!QFile::remove(strDstPath))
        {
            CLOG::Out("文件 %s 删除失败!可能被占用!", strDstPath.toUtf8().data());
            emit finished(false);
            return false;
        }
    }

    if (!createDirectory(strDstPath, true))
    {
        CLOG::Out("文件 %s 需要的目录创建失败!", strDstPath.toUtf8().data());
        emit finished(false);
        return false;
    }

    bool bRes = true;

#ifdef COPY_SYS_CMD
    if (!copyFileSystem(strSrcPath, strDstPath))
    {
        bRes = false;
        CLOG::Out(strSrcPath + " -> " + strDstPath + "复制失败!");
    }
#else
        QFile source(strSrcPath);
        QString dest = strDstPath;
        if (!source.copy(dest))
        {
            bRes = false;
            CLOG::Out(strSrcPath + " -> " + strDstPath + "复制失败! 错误原因：" + source.errorString());
        }
#endif

    emit finished(bRes);
    return bRes;
}

bool ProcessFile::copyFileSystem(const QString &strSrcPath, const QString &strDstPath)
{
    bool bRes = true;
#ifdef Q_OS_LINUX
        std::string command = "cp \"";
        command  += strSrcPath.toUtf8().data();
        command  += "\" \"";
        command  += strDstPath.toUtf8().data();     //cp /home/file1 /root/file2
        command  += "\"";
        bRes = system((char*)command.c_str())  != -1;
#else
    QFile source(strSrcPath);
    QString dest = strDstPath;
    if (!source.copy(dest))
    {
        bRes = false;
        CLOG::Out(strSrcPath + " -> " + strDstPath + "复制失败! 错误原因：" + source.errorString());
    }
#endif
    return bRes;
}
