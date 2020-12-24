#ifndef Q_CONFIGURE_H
#define Q_CONFIGURE_H

#include <QString>
#include <QCoreApplication>

typedef struct _config_s
{
    bool bRemItem;
    bool bRemPath;
    QStringList lDirPaths;
    QStringList lItemPaths;
    QStringList lSuffixs;

    _config_s()
    {
        bRemItem = true;
        bRemPath = true;
        lDirPaths.clear();
        lItemPaths.clear();
        lSuffixs.clear();
    }

}config_s;


QString GetConfigPath();

int ReadConfigFile(const QString& stConfigPath, config_s& _cfg);
int WriteConfigFile(const QString& stConfigPath, config_s& _cfg);

#endif
