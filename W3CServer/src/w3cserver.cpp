#include <QWebSocketServer>
#include <QWebSocket>
#include <QDebug>
#include <QFile>
#include <QSslCertificate>
#include <QSslKey>
#include <QThreadPool>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPointer>

#include "w3cserver.h"
#include "jsonrequestparser.h"
#include "request-handler/processrequesttask.h"
#include "qjsonwebtoken.h"
#include "jwt-utility/visstokenvalidator.h"
#include "VSSSignalinterface/vsssignalinterfaceimpl.h"
#include "VSSSignalinterface/vsssignalinterface.h"
#include "messaging/websocketwrapper.h"

QT_USE_NAMESPACE

class W3CServer;

W3CServer::W3CServer(quint16 port,bool usesecureprotocol, bool debug, QObject *parent) : QObject(parent),
    m_pWebSocketServer(0),
    m_clients(),
    m_debug(debug),
    m_secure(usesecureprotocol)
{
     QThreadPool::globalInstance() -> setMaxThreadCount(100);
    if (usesecureprotocol)
    {
        m_pWebSocketServer = new QWebSocketServer(QStringLiteral("W3CServer"),
                QWebSocketServer::SecureMode,
                this);
        // m_pWebSocketServer ->
        QSslConfiguration sslConfiguration;

        QFile keyFile(QStringLiteral(":/server.key"));
        QFile certFile(QStringLiteral(":/server.crt"));

        certFile.open(QIODevice::ReadOnly);
        const QByteArray bytes = certFile.readAll();
        QJsonWebToken e;
        qDebug() << "cert file length : " + QString::number(bytes.length());
        QSslCertificate certificate(bytes, QSsl::Pem);

        keyFile.open(QIODevice::ReadOnly);
        const QByteArray passphrase("6610");
        QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem,QSsl::PrivateKey,passphrase);

        certFile.close();
        keyFile.close();

        sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
        sslConfiguration.setLocalCertificate(certificate);
        sslConfiguration.setPrivateKey(sslKey);
        sslConfiguration.setProtocol(QSsl::TlsV1SslV3);

        m_pWebSocketServer->setSslConfiguration(sslConfiguration);
    }
    else
    {
        m_pWebSocketServer = new QWebSocketServer(QStringLiteral("W3CServer Test"),QWebSocketServer::NonSecureMode,this);
    }
    if (m_pWebSocketServer->listen(QHostAddress::Any, port))
    {
        if (m_debug)
        {
            qDebug() << "W3CServer is listening on port " << port;
        }

        //Connect QWebSocketServer newConnection signal with W3cServer slot onNewConnection
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this,&W3CServer::onNewConnection);
        //Connect QWebsocketServer signal with W3CServer signal closed
       // connect(m_pWebSocketServer,&QWebSocketServer::closed, this, &W3CServer::closed);
        //SSL error handler
        connect(m_pWebSocketServer, &QWebSocketServer::sslErrors,
                this, &W3CServer::onSslErrors);
    }

    // TODO: select implementation based on application configuration

    const QString vssFile = "/etc/vss_rel_1.json";
    m_vsssInterface = QSharedPointer<VSSSignalInterfaceImpl>(new VSSSignalInterfaceImpl(vssFile));
}

W3CServer::~W3CServer()
{
    m_pWebSocketServer->close();
    //clean out all connected clients
    qDeleteAll(m_clients.begin(),m_clients.end());
    m_vsssInterface.clear();
}

void W3CServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    // pSocket ->
    qDebug() << " attempting to connect ";

    // Connect socket textMessageReceived signal with server processTextMessage slot
    connect(pSocket, &QWebSocket::textMessageReceived, this, &W3CServer::processTextMessage);
    // Connect socket disconnected signal with server socketDisconnected slot
    connect(pSocket, &QWebSocket::disconnected, this, &W3CServer::socketDisconnected);

    // add socket to list of clients
    m_clients.insert(pSocket, new QMutex());
}

void W3CServer::processTextMessage(const QString& message)
{


    if (m_debug)
    {
        qDebug() << "Message recieved: " << message;
    }


    QWebSocket *zeClient = qobject_cast<QWebSocket *> (sender());

    if (m_clients.contains(zeClient)){
        // we need a mutex per client .
        QMutex* mutex = m_clients.find(zeClient).value();
        QPointer<WebSocketWrapper> socketWrapper = new WebSocketWrapper(zeClient, mutex);
        startRequestProcess(socketWrapper, message);
    }
    else
    {
        qDebug() << "fatal connection error, websocket client not found ";
    }

}

void W3CServer::socketDisconnected()
{
    QWebSocket *zeClient = qobject_cast<QWebSocket *> (sender());
    if (m_debug)
    {
        qDebug() << " socket disconnected: " << zeClient;
    }

    //remove from client list and delete from heap
    if (zeClient)
    {
        m_clients.remove(zeClient);
        zeClient->deleteLater();
    }
}

void W3CServer::onSslErrors(const QList<QSslError> &)
{
    qDebug() << "Ssl error occurred";
}


void W3CServer::startRequestProcess(WebSocketWrapper* sw, const QString& message)
{
    ProcessRequestTask* requesttask = new ProcessRequestTask(sw, m_vsssInterface, message, true);
    // QThreadPool takes ownership and deletes 'requesttask' automatically

    QThreadPool::globalInstance()->start(requesttask);
}
