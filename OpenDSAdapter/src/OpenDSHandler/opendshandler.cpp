#include "opendshandler.h"
#include <QPointer>
#include <QSettings>
#include <QDebug>
#include <QStringBuilder>
#include <QTcpSocket>
#include <QTime>
#include <QCoreApplication>
#include <QDomDocument>

OpenDSHandler::OpenDSHandler(QObject *parent) : QObject(parent)
{
    qDebug() << "handler executing";

    //Initiate socket and connect to simulation server (note that IP needs to be changed to match your server IP)
    m_Socket = new QTcpSocket(this);
    connect(m_Socket, &QTcpSocket::connected,this, &OpenDSHandler::connected);
    connect(m_Socket, &QTcpSocket::disconnected,this, &OpenDSHandler::disconnected);
    connect(m_Socket, &QTcpSocket::bytesWritten,this, &OpenDSHandler::bytesWritten);
    connect(m_Socket, &QTcpSocket::readyRead,this, &OpenDSHandler::readyRead);
    connect(m_Socket, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), this, &OpenDSHandler::socketError);

    // Reading server settings from settings file
    QPointer<QSettings> settings = new QSettings();
    settings->beginGroup("OpenDSHandler");
    settings->beginGroup("servers");
    m_server_ip = settings->value("server_ip").toString();
    m_server_port = settings->value("server_port").toInt();
    m_delay_sec = settings->value("delay_sec").toInt();


    qDebug() << "server ip: " << m_server_ip;
    qDebug() << "server port: " << m_server_port;

    m_Socket->connectToHost(m_server_ip, m_server_port);
}

void OpenDSHandler::reconnect()
{
    //if connection to OpenDS Server was lost we wait for x seconds and then retries!
    delay(m_delay_sec);
    // "times up, should now retry....";
    m_Socket->connectToHost(m_server_ip, m_server_port);
}

void OpenDSHandler::delay(int delay)
{
    QTime delayTime = QTime::currentTime().addSecs(delay);
    while (QTime::currentTime() < delayTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

void OpenDSHandler::xmlParser(QString xmlData)
{
//    qDebug() << "All OpenDS data: " << xmlData;

    //Get your xml into xmlText(you can use QString instead og QByteArray)
    QDomDocument doc;
    doc.setContent(xmlData);

    //Parse data
    QDomNodeList speed=doc.elementsByTagName("speed");
    QDomNodeList rpm=doc.elementsByTagName("actualRpm");
    QDomNodeList pressedState=doc.elementsByTagName("pressedState"); // GASPEDAL + BRAKEPEDAL
    QDomNodeList steerAngle=doc.elementsByTagName("steerAngle");
    QDomNodeList headlights=doc.elementsByTagName("headlights");
    QDomNodeList currentGear=doc.elementsByTagName("currentGear");
    QDomNodeList running=doc.elementsByTagName("running");
    QDomNodeList actualRpm=doc.elementsByTagName("actualRpm");
    QDomNodeList currentConsumption=doc.elementsByTagName("currentConsumption");
    QDomNodeList maxAmount=doc.elementsByTagName("maxAmount");
    QDomNodeList actualAmount=doc.elementsByTagName("actualAmount");
    QDomNodeList latitude=doc.elementsByTagName("latitude");
    QDomNodeList longitude=doc.elementsByTagName("longitude");
    QDomNodeList altitude=doc.elementsByTagName("altitude");
    QDomNodeList orientation=doc.elementsByTagName("orientation");
    QDomNodeList rise=doc.elementsByTagName("rise");
    QDomNodeList accelerationLateral=doc.elementsByTagName("accelerationLateral");
    QDomNodeList rotation=doc.elementsByTagName("rotation");
    QDomNodeList accelerationRotation=doc.elementsByTagName("accelerationRotation");
    QDomNodeList acceleration=doc.elementsByTagName("acceleration");
    QDomNodeList handBrakeOn=doc.elementsByTagName("handBrakeOn");
    QDomNodeList cruiseControlActivated=doc.elementsByTagName("cruiseControlActivated");
    QDomNodeList cruiseControlIncrease=doc.elementsByTagName("cruiseControlIncrease");
    QDomNodeList cruiseControlDecrease=doc.elementsByTagName("cruiseControlDecrease");

    //notify listners for valueChanged
    emit valueChanged(VSSSignalInterfaceImpl::Speed, speed.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::RPM, rpm.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::GasPedal, pressedState.at(1).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::BrakePedal, pressedState.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::SteerAngle, steerAngle.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::Headlights, headlights.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::EngineRunning, running.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::CurrentFuelConsumption, currentConsumption.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::FuelTankMax, maxAmount.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::FuelTankActual, actualAmount.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::PositionLatitude, latitude.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::PositionLongitude, longitude.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::PositionAltitude, altitude.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::Orientation, orientation.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::Rise, rise.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::AccelerationLateral, accelerationLateral.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::Rotation, rotation.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::AccelerationRotation, accelerationRotation.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::Acceleration, acceleration.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::CruiseControl, cruiseControlActivated.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::CruiseControlUp, cruiseControlIncrease.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::CruiseControlDown, cruiseControlDecrease.at(0).toElement().text());
    emit valueChanged(VSSSignalInterfaceImpl::HandBrake, handBrakeOn.at(0).toElement().text());


    //DN DEBUG-----------------------------
   setValue(VSSSignalInterfaceImpl::HandBrake, "true");
   // setValue(VSSSignalInterfaceImpl::CruiseControl, "true");
   // setValue(VSSSignalInterfaceImpl::CruiseControlUp, "15");
   // setValue(VSSSignalInterfaceImpl::CruiseControlUp, "35");
   // setValue(VSSSignalInterfaceImpl::CruiseControlDown, "25");
    //--------------------------------------

}

void OpenDSHandler::connected()
{
    qDebug() << "connected to OpenDS Server, sending subscribe message";

    QByteArray message = getSubscribeMessage();

    qDebug() << message;

    m_Socket->write(message);
}

void OpenDSHandler::disconnected()
{
    qDebug() << "disconnected... reconnecting";
    reconnect();
}

void OpenDSHandler::bytesWritten(qint64 bytes)
{
    qDebug() << bytes << " bytes written...";
}

void OpenDSHandler::readyRead()
{
    // read the data from the socket
   xmlParser(m_Socket->readAll());
}

void OpenDSHandler::socketError(QAbstractSocket::SocketError error)
{
//    qDebug() << "OpenDSHandler socketError: something went wrong...." << error;

    // check if socket is still connected or if we need to reconnect!
    if(!(m_Socket->state() == QTcpSocket::ConnectedState))
    {
//        qDebug() << "Trying to recover";
        reconnect();
    }
}

void OpenDSHandler::setValue(VSSSignalInterfaceImpl::CarSignalType signal, QString value)
{
    QByteArray message = getSetMessage(signal, value);

    //qDebug() << message;

    m_Socket->write(message);
}

QByteArray OpenDSHandler::getSubscribeMessage()
{
    // Reading the SubscribeMessage from settings file
    QPointer<QSettings> settings = new QSettings();

    settings->beginGroup("SignalServer");

    settings->beginGroup("subscribe");
    QString unsubscribe = settings->value("unsubscribe").toString();
    QString setupdateinterval = settings->value("setupdateinterval").toString();
    QString establishconnection = settings->value("establishconnection").toString();
    settings->endGroup();

    qDebug() << "unsubscribe: " << unsubscribe;
    qDebug() << "setupdateinterval: " << setupdateinterval;
    qDebug() << "establishconnection: " << establishconnection;

    int size = settings->beginReadArray("signal");

    //
    // Constructing readSubscribeMessage from settings file
    //
    QString message = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r";
    message = message % "<Message>""\r";
    // First unsubscribe previous
    QString event = "<Event Name=\"Unsubscribe\">" % unsubscribe % "</Event>\r";
    message = message % event;

    // Then add all new signals to subscribe to
    for (int i = 0; i < size; i++)
    {
        settings->setArrayIndex(i);
        QString value = settings->value("name").toString();
        event = "<Event Name=\"Subscribe\">" % value % "</Event>\r";
        message = message % event;

        qDebug() << "getSubscribeMessage : size: " << size << " value: " << value;
    }

    // Finally add update interval
    event = "<Event Name=\"SetUpdateInterval\">" % setupdateinterval % "</Event>\r";
    message = message % event;
    event = "<Event Name=\"EstablishConnection\">" % establishconnection % "</Event>\r";
    message = message % event;

    message = message % "</Message>\r";

    settings->endArray();
    settings->endGroup();

    return message.toLocal8Bit();
}

#if 1 //DN DEBUG
QByteArray OpenDSHandler::getSetMessage(VSSSignalInterfaceImpl::CarSignalType signal, QString value)
{
    QString entry = "";

    QStringList keyList = QStringList() << "/root/thisVehicle/interior/cockpit/cruiseControl/Properties/cruiseControlActivated"
                                        << "/root/thisVehicle/interior/cockpit/handBrake/Properties/handBrakeOn"
                                        << "/root/thisVehicle/interior/cockpit/cruiseControl/Properties/cruiseControlIncrease";
                                        //<< "/root/thisVehicle/interior/cockpit/cruiseControl/Properties/cruiseControlDecrease";

    QStringList valueList = QStringList() << "true"
                                          << "false"
                                          << "1";
                                          //<< "0";


    // Constructing XML request to OpenDS
    QString message = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r";
    message = message % "<Message>""\r";

    for (int i = 0; i < keyList.size(); ++i)
    {
        QString key = "SetValue";

        QString event = "<Event Name=\"" % key % "\" Value=\"" % valueList[i] % "\">" % keyList[i] % "</Event>\r";
        message = message % event;
    }

    message = message % "</Message>\r";

    return message.toLocal8Bit();
}

#else //#if 0 DN DEBUG
QByteArray OpenDSHandler::getSetMessage(VSSSignalInterfaceImpl::CarSignalType signal, QString value)
{
    QString entry = "";

    switch (signal)
    {
        case VSSSignalInterfaceImpl::CruiseControl:
            entry = "/root/thisVehicle/interior/cockpit/cruiseControl/Properties/cruiseControlActivated";
            break;

        case VSSSignalInterfaceImpl::CruiseControlUp:
            entry = "/root/thisVehicle/interior/cockpit/cruiseControl/Properties/cruiseControlIncrease";
            break;

        case VSSSignalInterfaceImpl::CruiseControlDown:
            entry = "/root/thisVehicle/interior/cockpit/cruiseControl/Properties/cruiseControlDecrease";
            break;

        case VSSSignalInterfaceImpl::HandBrake:
            entry = "/root/thisVehicle/interior/cockpit/handBrake/Properties/handBrakeOn";
            break;

        default:
            break;
    }

    // Constructing XML request to OpenDS
    QString message = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r";
    message = message % "<Message>""\r";

    QString key = "SetValue";

    QString event = "<Event Name=\"" % key % "\" Value=\"" % value % "\">" % entry % "</Event>\r";
    message = message % event;

    message = message % "</Message>\r";

    return message.toLocal8Bit();
}
#endif
