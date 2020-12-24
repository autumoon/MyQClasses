#ifndef CLOG_H
#define CLOG_H

#include <QObject>
#include <QString>
#include <QMutex>

class CLOG : public QObject
{
    Q_OBJECT
public:
    explicit CLOG(QObject *parent = nullptr);

    //如果没有指定，则为程序所在路径下的以日期命名的文件
    static QString  GetLogFile();
    static short  SetLogFile(QString strLogFile = "");
    static short  ViewLogFile();

    //-前缀-
    //----如果多个进程往同一个文件输出日志，可以为每个进程设置一个前缀
    static short  SetPrefix(QString strPrefix);

    //-清理日志-
    static short  Clean();
    static short  End(const char* pInfo = "\n");

    static short  Out(const QString& strInfo);
    static short  Out(const char* strFormat = nullptr, ...);

protected:
    static QString  s_strLogFile;
    static QString  s_strLogPrefix;
    static QMutex s_mutex;

signals:

public slots:

private:
    //-获取日志字符串,可以另外-
    static QString  sOutV(QString strType, const char* strFormat = nullptr, va_list valist = nullptr);
    static QString  sOut0(QString strType, const char* strFormat = nullptr,...);
    static QString  sOut (const char* strFormat = nullptr,...);

    //-将日志信息输出到文件-
    static short  OutV(QString strType, const char* strFormat = nullptr, va_list valist = nullptr);
    static short  Out0(QString strType, const char* strFormat = nullptr,...);

};

#endif // CLOG_H
