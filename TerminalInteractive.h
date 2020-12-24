#ifndef TERMINALINTERACTIVE_H
#define TERMINALINTERACTIVE_H

#include <QObject>
#include <QProcess>
#include <QThread>
#include <QMutex>

class TerminalInteractive : public QObject
{
    Q_OBJECT
public:
    explicit TerminalInteractive(const QString &strPwd, QObject *parent = nullptr);

public:
    void Command(const QString& strCmd, const bool& bWaitForFinished = true);

Q_SIGNALS:
    void readyReadStandardOutput(const QString& strStandardOutput);
    void readyReadStandardError(const QString& strStandardError);
    void finished();

private slots:
    void on_readoutput();
    void on_readerror();

private:
    void initialize(const QString& strPwd = "");

private:
    QString m_strPwd;
    static QMutex m_mutex;
    static QProcess *m_pCmd;
};

//启用线程执行
class TerminateWorker : public QObject
{
    Q_OBJECT
public slots:
    void doWork(const QString& strPwd, const QString& strCmd, const bool& bWaitForFinished)
    {
        /*耗时操作*/
        TerminalInteractive *ti = new TerminalInteractive(strPwd);
        connect(ti, SIGNAL(readyReadStandardOutput(const QString&)), this, SIGNAL(readyReadStandardOutput(const QString&)));
        connect(ti, SIGNAL(readyReadStandardError(const QString&)), this, SIGNAL(readyReadStandardError(const QString&)));
        connect(ti, SIGNAL(finished()), this, SIGNAL(finished()));
        ti->Command(strCmd, bWaitForFinished);
    }

Q_SIGNALS:
    void readyReadStandardOutput(const QString& strStandardOutput);
    void readyReadStandardError(const QString& strStandardError);
    void finished();
};

class ThreadTerminate : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    ThreadTerminate()
    {
        TerminateWorker *worker = new TerminateWorker;
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(worker, SIGNAL(readyReadStandardOutput(const QString&)), this, SIGNAL(readyReadStandardOutput(const QString&)));
        connect(worker, SIGNAL(readyReadStandardError(const QString&)), this, SIGNAL(readyReadStandardError(const QString&)));
        connect(worker, SIGNAL(finished()), this, SIGNAL(finished()));
        connect(this, SIGNAL(start_command(const QString&, const QString&, const bool&)), worker, SLOT(doWork(const QString&, const QString&, const bool&)));
        workerThread.start();
    }

    ~ThreadTerminate()
    {
        workerThread.quit();
        workerThread.wait();
    }

public slots:
    void Command(const QString& strPwd, const QString& strCmd, const bool& bWaitForFinished)
    {
        emit start_command(strPwd, strCmd, bWaitForFinished);
    }

Q_SIGNALS:
    void start_command(const QString& strPwd, const QString& strCmd, const bool& bWaitForFinished);
    void readyReadStandardOutput(const QString& strStandardOutput);
    void readyReadStandardError(const QString& strStandardError);
    void finished();
};

#endif // TERMINALINTERACTIVE_H
