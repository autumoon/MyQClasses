#include "JsonParser.h"
#include <QFile>
#include "Log.h"

bool JsonParser::CheckStandardResponse(const QByteArray &baReply, QString& strInfo)
{
    QJsonParseError jsonpe;
    QJsonDocument json_doc = QJsonDocument::fromJson(baReply, &jsonpe);
    if (jsonpe.error == QJsonParseError::NoError)
    {
        if (json_doc.isObject())
        {
            QJsonObject obj_json = json_doc.object();
            //检查flag
            if (obj_json.contains("flag"))
            {
                QJsonValue j_flag = obj_json.take("flag");

                //如果flag为fasle需要输出原因

                bool bFlag = j_flag.toBool();

                if (!bFlag)
                {
                    //标准响应为false的时候,输出响应结果
                    if (obj_json.contains("message"))
                    {
                        QJsonValue j_info = obj_json.take("message");
                        strInfo = j_info.toString();
                    }
                }

                return  bFlag;
            }
            else
            {
                CLOG::Out("no flag key!");
            }
        }
        else
        {
            CLOG::Out("error, should json object");
        }
    }
    else
    {
        CLOG::Out(jsonpe.errorString().toUtf8().data());
    }

    CLOG::Out(baReply.data());

    return false;
}

bool JsonParser::SaveJsonByteArray(const QByteArray &baJson, const QString &strJsonSavePath)
{
    QJsonParseError jsonpe;
    QJsonDocument json_doc = QJsonDocument::fromJson(baJson, &jsonpe);
    if (jsonpe.error == QJsonParseError::NoError)
    {
        QFile file(strJsonSavePath);
        if(!file.open(QFile::WriteOnly | QIODevice::Text))
        {
            CLOG::Out("%s保存失败,无法创建文件!", strJsonSavePath.toUtf8().data());
            return false;
        }

         file.write(json_doc.toJson());
         return true;
    }

    CLOG::Out("%s文件保存失败!失败原因为%s", strJsonSavePath.toUtf8().data(), jsonpe.errorString().toUtf8().data());

    return false;
}

bool JsonParser::UpdateJsonValue(QByteArray &baJson, const QString &strKey, const QJsonValue &value)
{
    QJsonParseError jsonpe;
    QJsonDocument json_doc = QJsonDocument::fromJson(baJson, &jsonpe);
    if (jsonpe.error == QJsonParseError::NoError)
    {
        if (json_doc.isObject())
        {
            QJsonObject obj_json = json_doc.object();

            //检查是否包含字段
            if (obj_json.contains(strKey))
            {
                //QJsonValueRef ref = obj_json.find(strKey).value();
                obj_json[strKey] = value;
                json_doc.setObject(obj_json);

                baJson = json_doc.toJson();

                return true;
            }
        }
    }

    CLOG::Out("json文件%s更新字段%s的值为%s失败！", baJson.data(), strKey.toUtf8().data(), value.toString().toUtf8().data());

    return false;
}

int JsonParser::GetAccessToken(const QByteArray &data, QString &strToken)
{
    QJsonParseError jsonpe;
    QJsonDocument json_doc = QJsonDocument::fromJson(data, &jsonpe);
    if (jsonpe.error == QJsonParseError::NoError)
    {
        if (json_doc.isObject())
        {
            QJsonObject obj_json = json_doc.object();
            //检查flag
            if (obj_json.contains("flag"))
            {
                QJsonValue j_flag = obj_json.take("flag");
                if (j_flag.toBool() && obj_json.contains("data"))
                {
                    QJsonValue j_data = obj_json.take("data");

                    if (j_data.isObject())
                    {
                        QJsonObject o_data = j_data.toObject();
                        if (o_data.contains("access_token"))
                        {
                            QJsonValue j_data = o_data.take("access_token");
                            strToken = "Bearer " +  j_data.toString();

                            return 0;
                        }
                    }
                }

                if (!j_flag.toBool() && obj_json.contains("message"))
                {
                    QJsonValue j_data = obj_json.take("message");
                    CLOG::Out("message = %s", j_data.toString().toUtf8().data());
                }
            }
            else
            {
                CLOG::Out("不包含flag字段!");
            }
        }
        else
        {
            CLOG::Out("error, should json object");
        }
    }
    else
    {
        CLOG::Out(jsonpe.errorString());
    }

    CLOG::Out(data.data());

    return -1;
}

int JsonParser::GetFilesFromJson(const QByteArray &data, QStringList &lDstFiles, QString strRootDir /*= ""*/)
{
    if (!strRootDir.isEmpty()) DelSlashIfNeeded(strRootDir);
    //假设已经获取到了json数据
    QJsonParseError complex_json_error;
    QJsonDocument complex_parse_doucment = QJsonDocument::fromJson(data, &complex_json_error);
    if (complex_json_error.error == QJsonParseError::NoError)
    {
        if (complex_parse_doucment.isObject())
        {
            QJsonObject obj_json = complex_parse_doucment.object();

            //检查flag
            if (obj_json.contains("flag"))
            {
                QJsonValue j_flag = obj_json.take("flag");
                if (j_flag.toBool() && obj_json.contains("data"))
                {
                    QJsonValue j_data = obj_json.take("data");
                    if (j_data.isObject())
                    {
                        QJsonObject obj_data = j_data.toObject();
                        if (obj_data.contains("list"))
                        {
                            QJsonValue  v_list = obj_data.take("list");
                            if (v_list.isArray())
                            {
                                QJsonArray arrList = v_list.toArray();

                                int nArrCount = arrList.size();

                                if (nArrCount >= 1)
                                {
                                    QJsonObject objList = arrList[0].toObject();

                                    if (objList.contains("otherInfo"))
                                    {
                                        QJsonValue  valFiles = objList.take("otherInfo");
                                        if (valFiles.isArray())
                                        {
                                            QJsonArray arrFiles = valFiles.toArray();
                                            int nFileCount = arrFiles.size();
                                            for (int i = 0; i < nFileCount; ++i)
                                            {
                                                lDstFiles.push_back(strRootDir + arrFiles[i].toString());
                                            }

                                            return 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                CLOG::Out("不包含flag字段!");
            }
        }
        else
        {
            CLOG::Out("error, should json object");
        }
    }
    else
    {
        CLOG::Out(complex_json_error.errorString());
    }

    CLOG::Out("获取列表失败json内容为%s", data.data());

    return -1;
}

int JsonParser::GetHostResRootDir(const QByteArray &data, QString &strRootDir)
{
    //home/gis/resRootDir/基础数据网数据/导线测量/测试/2020-04-14/基础数据网导线1"

    QJsonParseError jsonpe;
    QJsonDocument json_doc = QJsonDocument::fromJson(data, &jsonpe);
    if (jsonpe.error == QJsonParseError::NoError)
    {
        if (json_doc.isObject())
        {
            QJsonObject obj_json = json_doc.object();
            //检查flag
            if (obj_json.contains("flag"))
            {
                QJsonValue j_flag = obj_json.take("flag");
                if (j_flag.toBool() && obj_json.contains("data"))
                {
                    QJsonValue j_data = obj_json.take("data");

                    strRootDir = j_data.toString();

                    return 0;
                }
            }
            else
            {
                CLOG::Out("不包含flag字段!");
            }
        }
        else
        {
           CLOG::Out( "error, should json object");
        }
    }
    else
    {
       CLOG::Out("error:"  + jsonpe.errorString());
    }

    CLOG::Out(data.data());

    return -1;
}

int JsonParser::JsonFile2ByteArray(const QString &strJsonPath, QByteArray &baDst)
{
    QFile file(strJsonPath);
    if(!file.open(QIODevice::ReadOnly))
    {
        CLOG::Out("文件%s打开失败!", strJsonPath.toUtf8().data());
        return -1;
    }
    baDst = file.readAll();
    file.close();

    return 0;
}

int JsonParser::ParseStringList(QByteArray &baJson, QStringList &lValues)
{
    QJsonParseError jsonpe;
    QJsonDocument json_doc = QJsonDocument::fromJson(baJson, &jsonpe);
    if (jsonpe.error == QJsonParseError::NoError)
    {
        if (json_doc.isArray())
        {
            QJsonArray arr = json_doc.array();

            int nCount = arr.size();
            for (int i = 0; i < nCount; ++i)
            {
                lValues.push_back(arr[i].toString());
            }

            return nCount;
        }
    }

    return -1;
}

QByteArray JsonParser::StringList2ByteArray(const QStringList &lValues, const bool bToUtf8)
{
    int nValues = lValues.size();

    QJsonArray arrList;

    if (bToUtf8)
    {
        for (int i = 0; i < nValues; ++i)
        {
            arrList.push_back(QJsonValue(lValues[i].toUtf8().data()));
        }
    }
    else
    {
        for (int i = 0; i < nValues; ++i)
        {
            arrList.push_back(QJsonValue(lValues[i]));
        }
    }

    return QJsonDocument(arrList).toJson();
}

QByteArray JsonParser::StringList2ByteArray(const QStringList &lKeys, const QStringList &lValues, const bool bToUtf8 /*= false*/)
{
    int nKeys = lKeys.size();
    int nValues = lValues.size();
    int nParis = qMin(nKeys, nValues);

    QJsonObject jo;

    if (bToUtf8)
    {
        for (int i = 0; i < nParis; ++i)
        {
            jo.insert(lKeys[i], QJsonValue(lValues[i].toUtf8().data()));
        }
    }
    else
    {
        for (int i = 0; i < nParis; ++i)
        {
            jo.insert(lKeys[i], QJsonValue(lValues[i]));
        }
    }

    return QJsonDocument(jo).toJson();
}

int JsonParser::ParseRootKeyValue(QByteArray& baJson, const QString& strKey, QString& strResult)
{
    QJsonParseError jsonpe;
    QJsonDocument json_doc = QJsonDocument::fromJson(baJson, &jsonpe);
    if (jsonpe.error == QJsonParseError::NoError)
    {
        if (json_doc.isObject())
        {
            QJsonObject obj_json = json_doc.object();

            //检查是否包含字段
            if (obj_json.contains(strKey))
            {
                //QJsonValueRef ref = obj_json.find(strKey).value();
                strResult = obj_json[strKey].toString();

                return 0;
            }
        }
    }

    return -1;
}
