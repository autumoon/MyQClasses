#ifndef PARSE_JSON_H
#define PARSE_JSON_H

//json
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QStringList>

class JsonParser
{
public:
    //检查标准响应
    static bool CheckStandardResponse(const QByteArray& baReply, QString& strInfo);
    //保存到json文件
    static bool SaveJsonByteArray(const QByteArray& baJson, const QString& strJsonSavePath);
    //更新json中某个字段的值，只针对简单object类型的json数据
    static bool UpdateJsonValue(QByteArray& baJson, const QString& strKey, const QJsonValue& value);
    //从json中获取token
    static int GetAccessToken(const QByteArray& data, QString& strToken);
    //从json中获取文件列表
    static int GetFilesFromJson(const QByteArray &data, QStringList &lDstFiles, QString strRootDir = "");
    //从json中获取资源根路径
    static int GetHostResRootDir(const QByteArray &data, QString& strRootDir);
    //将json文件转化为BytaArray
    static int JsonFile2ByteArray(const QString& strJsonPath, QByteArray& baDst);
    //读取根键值
    static int ParseStringList(QByteArray& baJson, QStringList& lValues);
    static int ParseRootKeyValue(QByteArray& baJson, const QString& strKey, QString& strResult);

    //将key\vlaue对转换为json
    static QByteArray StringList2ByteArray(const QStringList& lValues, const bool bToUtf8 = false);
    static QByteArray StringList2ByteArray(const QStringList& lKeys, const QStringList& lValues, const bool bToUtf8 = false);
    //如果需要加上'/',注意传入的参数为引用
    static QString AddSlashIfNeeded(QString& strPath){if (!strPath.isEmpty() && !strPath.endsWith('/')) strPath += '/'; return strPath;}
    static QString DelSlashIfNeeded(QString& strPath){if (!strPath.isEmpty() && strPath.endsWith('/')) strPath = strPath.left(strPath.length() - 1); return strPath;}
};

#endif
