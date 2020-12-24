/*
  (C) 2011 dbzhang800#gmail.com
  update by autumoon 2020.05.27
*/
#ifndef MSGHANDLERWAPPER_H
#define MSGHANDLERWAPPER_H
#include <QtCore/QObject>

class MsgHandlerWapper:public QObject
{
    Q_OBJECT
public:
    static MsgHandlerWapper * instance();

signals:
    void message(QtMsgType, const QMessageLogContext &, const QString &);

private:
    MsgHandlerWapper();
    static MsgHandlerWapper * m_instance;
};

#endif // MSGHANDLERWAPPER_Hs
