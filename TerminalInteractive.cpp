#include "TerminalInteractive.h"
#include <QEventLoop>
#include <QDebug>

QMutex TerminalInteractive::m_mutex;
QProcess* TerminalInteractive::m_pCmd = nullptr;

TerminalInteractive::TerminalInteractive(const QString &strPwd, QObject *parent) : QObject(parent)
{
    if (m_pCmd == nullptr)
    {
        m_pCmd = new QProcess(this);
        connect(m_pCmd , SIGNAL(readyReadStandardOutput()) , this , SLOT(on_readoutput()));
        connect(m_pCmd , SIGNAL(readyReadStandardError()) , this , SLOT(on_readerror()));
        initialize(strPwd);
    }
}

void TerminalInteractive::initialize(const QString &strPwd)
{

#if defined (Q_OS_LINUX)

    m_pCmd->start("bash");
    m_pCmd->waitForStarted();
    m_pCmd->write("sudo -S pwd\n");
    m_pCmd->write((strPwd + '\n').toUtf8().data());

#elif defined (Q_OS_WINDOWS)

    Q_UNUSED(strPwd)
    m_pCmd->start("cmd");
    m_pCmd->waitForStarted();
    m_pCmd->write("chdir\n");

#endif
}

void TerminalInteractive::Command(const QString &strCmd, const bool &bWaitForFinished)
{
    m_mutex.lock();

#if defined (Q_OS_LINUX)

    emit readyReadStandardOutput(strCmd);

    QString strTermCmd = strCmd;

    bool bSudo = strTermCmd.contains("sudo ");

    if (bSudo)
    {
        //去掉"sudo "
        strTermCmd.replace("sudo ", "sudo -S ");
    }

    strTermCmd += '\n';

    m_pCmd->write(strTermCmd.toUtf8().data());


#elif defined (Q_OS_WINDOWS)

    QString strTermCmd = strCmd + '\n';
    m_pCmd->write(strTermCmd.toUtf8().data());

#endif

    if (bWaitForFinished)
    {
        m_pCmd->waitForFinished();
    }

    m_mutex.unlock();

    emit finished();
}

void TerminalInteractive::on_readoutput()
{
    QString strInfo = QString::fromLocal8Bit(m_pCmd->readAllStandardOutput().data());
    emit readyReadStandardOutput(strInfo);
}

void TerminalInteractive::on_readerror()
{
    QString strErr = QString::fromLocal8Bit(m_pCmd->readAllStandardOutput().data());
    emit readyReadStandardError(strErr);
}
