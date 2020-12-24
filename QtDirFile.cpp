#include "QtDirFile.h"
#include <QFileDialog>
#include <QDir>

QTDirFile::QTDirFile(QObject *parent) : QObject(parent)
{

}

QString QTDirFile::BrowseDir(QWidget *parent /*= nullptr*/, const QString& strInfo /*= "请选择目录"*/)
{
    //文件夹路径
    QString strDirPath = QFileDialog::getExistingDirectory(parent, strInfo, QDir::homePath());

    return strDirPath;
}

QString QTDirFile::OpenSuffixFile(QWidget *parent, QString strSuffix)
{
    LeftSuffixOnly(strSuffix);
    QString strFilter = strSuffix + "文件(*." + strSuffix + ");;所有文件（*.*);;";

    return QFileDialog::getOpenFileName(parent, tr("选择文件"), QDir::homePath(), strFilter);
}

int QTDirFile::getDirs(const QString &strDir, QStringList &lPathNames, bool bIncludeSub)
{
    //此处修改遍历文件夹地址
    QDir d(strDir);

    //列出文件,列出隐藏文件（在Unix下就是以.开始的为文件）,不列出符号链接（不支持符号连接的操作系统会忽略）
    d.setFilter(QDir::Hidden | QDir::NoSymLinks | QDir::AllDirs);

    //按文件大小排序，相反的排序顺序
    //d.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = d.entryInfoList();//返回这个目录中所有目录和文件的QFileInfo对象的列表

    while(!list.isEmpty())
    {
        QFileInfo tem= list.last();
        if(tem.isDir() && tem.fileName() != "." && tem.fileName() != "..")
        {
            lPathNames.push_back(tem.filePath());
            if (bIncludeSub)
            {
                QDir a(tem.filePath());
                list.pop_back();  //移除链表最后一项方便退回
                list.append(a.entryInfoList());
            }
            else
            {
                list.pop_back();  //移除链表最后一项方便退回
            }
        }
        else
        {
            list.pop_back();
        }
    }

    return lPathNames.size();
}

int QTDirFile::getFiles(const QString &strDir, QStringList& lPathNames, QString strSuffix, bool bIncludeSub)
{
    //此处修改遍历文件夹地址
    QDir d(strDir);

    //列出文件,列出隐藏文件（在Unix下就是以.开始的为文件）,不列出符号链接（不支持符号连接的操作系统会忽略）
    d.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks | QDir::AllDirs | QDir::NoDotAndDotDot);

    //按文件大小排序，相反的排序顺序
    //d.setSorting(QDir::Size | QDir::Reversed);
    QStringList nameFilters;
    QString strOnlySuffix = LeftSuffixOnly(strSuffix, true);
    nameFilters << "*." + strOnlySuffix;
    QFileInfoList list = d.entryInfoList();//返回这个目录中所有目录和文件的QFileInfo对象的列表

    while(!list.isEmpty())
    {
        QFileInfo tem= list.last();
        if(tem.isFile())
        {
            QString strPath = tem.filePath();
            QString strFileSuffix(strPath);
            if (strOnlySuffix == "*" || LeftSuffixOnly(strFileSuffix, true) == strOnlySuffix.toLower())
            {
                lPathNames.push_back(strPath);
            }

            list.pop_back();
        }
        else if (tem.isDir() && tem.fileName() != "." && tem.fileName() != ".." && bIncludeSub)
        {
            QDir a(tem.filePath());
            list.pop_back();
            list.append(a.entryInfoList());
        }
        else
        {
            list.pop_back();
        }
    }

    return lPathNames.size();
}

int QTDirFile::OpenMultiFiles(QWidget *parent, QString strClassName, QStringList &arrDstFiles, const int nSuffix, ...)
{
    va_list argp;
    va_start(argp, nSuffix);

    QStringList arrSuffixs;
    QString strSuffix;
    for (int i = 0; i < nSuffix; i++)
    {
        strSuffix = va_arg(argp, const char*);
        arrSuffixs.push_back(strSuffix);
    }
    va_end(argp);

    QString strFilter = strClassName + "(";
    if (strFilter.isEmpty())
    {
        strFilter = "指定类型(";
    }

    for (int i = 0; i < nSuffix; i++)
    {
        strFilter += "*." + LeftSuffixOnly(arrSuffixs[i]);

        if (i < nSuffix - 1)
        {
            strFilter += " ";
        }
    }
    strFilter += ");;";

    arrDstFiles = QFileDialog::getOpenFileNames(parent, tr("选择文件"), QDir::homePath(), strFilter);

    return arrDstFiles.size();
}

QString QTDirFile::LeftSuffixOnly(QString strPathOrFilter, bool bToLower /*= false*/)
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
