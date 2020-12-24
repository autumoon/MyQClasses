#include "MsgHandlerWapper.h"
#include <QtCore/QMetaType>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QCoreApplication>

/*
    connect(MsgHandlerWapper::instance(),
            SIGNAL(message(QtMsgType, const QMessageLogContext &, const QString &)),
            this, SLOT(outputMessage(QtMsgType, const QMessageLogContext &, const QString &)));

void outputMessage(QtMsgType, const QMessageLogContext &, const QString & msg)
{
    ui->textBrowser->append(msg);
}
*/

void static msgHandlerFunction(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    emit MsgHandlerWapper::instance()->message(type, context, msg);
//    QMetaObject::invokeMethod(MsgHandlerWapper::instance(), "message"
//                        , Q_ARG(QtMsgType, type),  Q_ARG(QMessageLogContext, context)
//                        , Q_ARG(QString, msg));
}

MsgHandlerWapper * MsgHandlerWapper::m_instance = 0;

MsgHandlerWapper * MsgHandlerWapper::instance()
{
    static QMutex mutex;
    if (!m_instance)
    {
        QMutexLocker locker(&mutex);
        if (!m_instance)
            m_instance = new MsgHandlerWapper;
    }

    return m_instance;
}

MsgHandlerWapper::MsgHandlerWapper()
    :QObject(qApp)
{
    qRegisterMetaType<QtMsgType>("QtMsgType");
    qInstallMessageHandler(msgHandlerFunction);
}
