#include "setgettestcase.h"
#include <QJsonDocument>
#include <QJsonObject>

SetGetTestCase::SetGetTestCase(QString testClientId, int& requestId, QObject *parent)
    : TestCaseBase(testClientId, requestId, parent)
{

}

void SetGetTestCase::startTest(QWebSocket* webSocket)
{
    m_webSocket = webSocket;
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &SetGetTestCase::onTextMessageReceived);

    QString subMess = GetVissTestDataJson::getTestDataString(requesttype::SET, QString::number(m_requestId++));

    m_webSocket->sendTextMessage(subMess);
}
void SetGetTestCase::forceStop()
{

}

void SetGetTestCase::onTextMessageReceived(QString message)
{
    DEBUG(m_testClientId,"Message received");
    TRACE(m_testClientId,message);

    QJsonParseError parseError;
    QJsonDocument  jsonDocument;
    QJsonObject    jsonObject;

    jsonDocument = QJsonDocument::fromJson(message.toUtf8(),&parseError);
    jsonObject = jsonDocument.object();
    if (parseError.error == QJsonParseError::NoError)
    {
        QString actionString =jsonObject["action"].toString();

        if(actionString == "set")
        {
            QString requestId = jsonObject["requestId"].toString();
            QJsonObject errorObject = jsonObject["error"].toObject();

            if (!errorObject.empty())
            {
                WARNING(m_testClientId,"Set Get failed.");
                QString errorMessage = errorObject["message"].toString();
                DEBUG(m_testClientId,QString("Error! Request ID : %1, Message : %2").arg(requestId, errorMessage));

                emit finished(false);
                return;
            }
            DEBUG(m_testClientId,"Successfully set value, now getting value.");
            QString subMess = GetVissTestDataJson::getTestDataString(requesttype::GET, QString::number(m_requestId++));
            m_webSocket->sendTextMessage(subMess);
        }
        else if(actionString == "get")
        {
            QString requestId = jsonObject["requestId"].toString();
            QJsonObject errorObject = jsonObject["error"].toObject();

            if (!errorObject.empty())
            {
                WARNING(m_testClientId,"Set Get failed.");
                QString errorMessage = errorObject["message"].toString();
                DEBUG(m_testClientId,QString("Error! Request ID : %1, Message : %2").arg(requestId, errorMessage));
                emit finished(false);
                return;
            }

            QJsonValue receivedValue = jsonObject.value("value");
            if(receivedValue.isUndefined())
            {
                WARNING(m_testClientId,"Get response doesn't contain any value.");
                emit finished(false);
                return;
            }

            // TODO Add check so that we set the same value as we set.
            // This would probably cause some errors though.. as two or more tests could do this at the same time
            // thus set different values and get differing results.

            INFO(m_testClientId,"Successfully set and got value.");
            emit  finished(true);
        }
        else
        {
            WARNING(m_testClientId,"Received incorrect action.");
            emit finished(false);
            return;
        }


    }
    else
    {
        emit  finished(false);
    }
}
