#include "Configure.h"

//json
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

#include <QFile>

QString GetConfigPath()
{
    QString strCorePath = QCoreApplication::applicationFilePath();

    return  strCorePath + ".json";

}

int ReadConfigFile(const QString& stConfigPath, config_s& _cfg)
{
    QFile file(stConfigPath);
    if(!file.open(QIODevice::ReadWrite)) {
        return -1;
    }

    QByteArray ba = file.readAll();
    QJsonParseError complex_json_error;
    QJsonDocument complex_parse_doucment = QJsonDocument::fromJson(ba, &complex_json_error);
    if (complex_json_error.error == QJsonParseError::NoError)
    {
        if (complex_parse_doucment.isObject())
        {
            //开始解析json对象
            QJsonObject jsonObject = complex_parse_doucment.object();
            if (jsonObject.contains("remItem"))
            {
                QJsonValue login_value = jsonObject.take("remItem");
                if (login_value.isBool())
                {
                    _cfg.bRemItem = login_value.toBool();
                }
            }

            if (jsonObject.contains("remPath"))
            {
                QJsonValue remPath_value = jsonObject.take("remPath");
                if (remPath_value.isBool())
                {
                    _cfg.bRemPath = remPath_value.toBool();
                }
            }


            if (jsonObject.contains("DstPaths"))
            {
                QJsonValue arrays_value = jsonObject.take("DstPaths");
                if (arrays_value.isArray())
                {
                    QJsonArray arrDstPathArray = arrays_value.toArray();
                    _cfg.lDirPaths.clear();
                    for (int i = 0; i < arrDstPathArray.size(); i++)
                    {
                     QJsonValue subArrayValue = arrDstPathArray.at(i);
                        if (subArrayValue.isString())
                         {
                            _cfg.lDirPaths.push_back(subArrayValue.toString());
                         }
                     }
                }
            }
            if (jsonObject.contains("ItemPaths"))
            {
                QJsonValue itemPath_value = jsonObject.take("ItemPaths");
                if (itemPath_value.isArray())
                {
                    QJsonArray itemArray = itemPath_value.toArray();
                    _cfg.lItemPaths.clear();
                    for (int i = 0; i < itemArray.size(); i++)
                    {
                     QJsonValue subArrayValue = itemArray.at(i);
                        if (subArrayValue.isString())
                         {
                            _cfg.lItemPaths.push_back(subArrayValue.toString());
                         }
                     }
                }
            }
            if (jsonObject.contains("Suffixs"))
            {
                QJsonValue suffix_value = jsonObject.take("Suffixs");
                if (suffix_value.isArray())
                {
                    QJsonArray suffixArray = suffix_value.toArray();
                    _cfg.lSuffixs.clear();
                    for (int i = 0; i < suffixArray.size(); i++)
                    {
                     QJsonValue subArrayValue = suffixArray.at(i);
                        if (subArrayValue.isString())
                         {
                            _cfg.lSuffixs.push_back(subArrayValue.toString());
                         }
                     }
                }
            }
        }
    }

    return  0;
}

int WriteConfigFile(const QString& stConfigPath, config_s& _cfg)
{
    QFile file(stConfigPath);
    if(!file.open(QIODevice::ReadWrite)) {
        return -1;
    }

    // 清空文件中的原有内容
    file.resize(0);

    // 使用QJsonObject对象插入键值对。
    QJsonObject jsonObject;
    jsonObject.insert("remItem", _cfg.bRemItem);
    jsonObject.insert("remPath", _cfg.bRemPath);

    QJsonArray jsonArrayAccountInfo;
    for(int i = 0; i < _cfg.lDirPaths.size(); i++)
    {
        QString stCurPath = _cfg.lDirPaths[i];
        jsonArrayAccountInfo.append(stCurPath);
    }
    jsonObject.insert("DstPaths", jsonArrayAccountInfo);

    QJsonArray jsonArrayItems;
    for(int i = 0; i < _cfg.lItemPaths.size(); i++)
    {
        QString stCurPath = _cfg.lItemPaths[i];
        jsonArrayItems.append(stCurPath);
    }
    jsonObject.insert("ItemPaths", jsonArrayItems);

    QJsonArray jsonArraySuffixs;
    for(int i = 0; i < _cfg.lSuffixs.size(); i++)
    {
        QString stCurPath = _cfg.lSuffixs[i];
        jsonArraySuffixs.append(stCurPath);
    }
    jsonObject.insert("Suffixs", jsonArraySuffixs);

    // 使用QJsonDocument设置该json对象
    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObject);

    // 将json以文本形式写入文件并关闭文件。
    file.write(jsonDoc.toJson());
    file.close();

    return  0;
}
