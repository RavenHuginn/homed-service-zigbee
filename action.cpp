#include <QtEndian>
#include "action.h"

void ActionObject::registerMetaTypes(void)
{
    qRegisterMetaType <Actions::Status>                     ("statusAction");
    qRegisterMetaType <Actions::PowerOnStatus>              ("powerOnStatusAction");
    qRegisterMetaType <Actions::Level>                      ("levelAction");
    qRegisterMetaType <Actions::ColorHS>                    ("colorHSAction");
    qRegisterMetaType <Actions::ColorXY>                    ("colorXYAction");
    qRegisterMetaType <Actions::ColorTemperature>           ("colorTemperatureAction");

    qRegisterMetaType <ActionsPTVO::ChangePattern>          ("ptvoChangePatternAction");
    qRegisterMetaType <ActionsPTVO::Pattern>                ("ptvoPatternAction");

    qRegisterMetaType <ActionsLUMI::Sensitivity>            ("lumiSensitivityAction");
    qRegisterMetaType <ActionsLUMI::Mode>                   ("lumiModeAction");
    qRegisterMetaType <ActionsLUMI::Distance>               ("lumiDistanceAction");
    qRegisterMetaType <ActionsLUMI::ResetPresence>          ("lumiResetPresenceAction");

    qRegisterMetaType <ActionsTUYA::Volume>                 ("tuyaVolumeAction");
    qRegisterMetaType <ActionsTUYA::Duration>               ("tuyaDurationAction");
    qRegisterMetaType <ActionsTUYA::Alarm>                  ("tuyaAlarmAction");
    qRegisterMetaType <ActionsTUYA::Melody>                 ("tuyaMelodyAction");
    qRegisterMetaType <ActionsTUYA::Sensitivity>            ("tuyaSensitivityAction");
    qRegisterMetaType <ActionsTUYA::DistanceMin>            ("tuyaDistanceMinAction");
    qRegisterMetaType <ActionsTUYA::DistanceMax>            ("tuyaDistanceMaxAction");
    qRegisterMetaType <ActionsTUYA::DetectionDelay>         ("tuyaDetectionDelayAction");
    qRegisterMetaType <ActionsTUYA::PowerOnStatus>          ("tuyaPowerOnStatusAction");

    qRegisterMetaType <ActionsPerenio::PowerOnStatus>       ("perenioPowerOnStatusAction");
    qRegisterMetaType <ActionsPerenio::ResetAlarms>         ("perenioResetAlarmsAction");
    qRegisterMetaType <ActionsPerenio::AlarmVoltageMin>     ("perenioAlarmVoltageMinAction");
    qRegisterMetaType <ActionsPerenio::AlarmVoltageMax>     ("perenioAlarmVoltageMaxAction");
    qRegisterMetaType <ActionsPerenio::AlarmPowerMax>       ("perenioAlarmPowerMaxAction");
    qRegisterMetaType <ActionsPerenio::AlarmEnergyLimit>    ("perenioAlarmEnergyLimitAction");
}

QByteArray ActionObject::writeAttributeRequest(const QByteArray &data)
{
    zclHeaderStruct header;
    writeArrtibutesStruct payload;

    header.frameControl = 0x00;
    header.transactionId = m_transactionId++;
    header.commandId = CMD_WRITE_ATTRIBUTES;

    payload.attributeId = qToLittleEndian <quint16> (m_attributeId);
    payload.dataType = m_dataType;

    return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload)).append(data);
}

QByteArray Actions::Status::request(const QVariant &data)
{
    zclHeaderStruct header;
    QString status = data.toString();

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = status == "toggle" ? 0x02 : status == "on" ? 0x01 : 0x00;

    return QByteArray(reinterpret_cast <char*> (&header), sizeof(header));
}

QByteArray Actions::PowerOnStatus::request(const QVariant &data)
{
    QList <QString> list = {"off", "on", "toggle", "previous"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0 || value > 2)
        value = 0xFF;

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray Actions::Level::request(const QVariant &data)
{
    zclHeaderStruct header;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;

    switch (data.type())
    {
        case QVariant::LongLong:
        {
            levelStruct payload;

            header.commandId = 0x00;
            payload.level = static_cast <quint8> (data.toInt() < 0xFE ? data.toInt() : 0xFE);
            payload.time = 0;

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::List:
        {
            levelStruct payload;
            QList <QVariant> list = data.toList();

            header.commandId = 0x00;
            payload.level = static_cast <quint8> (list.value(0).toInt() < 0xFE ? list.value(0).toInt() : 0xFE);
            payload.time = qToLittleEndian <quint16> (list.value(1).toInt());

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::String:
        {
            QString action = data.toString();

            if (action != "moveStop")
            {
                quint8 payload[2] = {static_cast <quint8> (action == "moveUp" ? 0x00 : 0x01), 0x55}; // TODO: check this
                header.commandId = 0x01;
                return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
            }

            header.commandId = 0x07;
            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header));
        }

        default:
            return QByteArray();
    }
}

QByteArray Actions::ColorHS::request(const QVariant &data)
{
    zclHeaderStruct header;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = 0x06;

    switch (data.type())
    {
        case QVariant::List:
        {
            colorHSStruct payload;
            QList <QVariant> list = data.toList();

            payload.colorH = static_cast <quint8> (list.value(0).toInt() < 0xFE ? list.value(0).toInt() : 0xFE);
            payload.colorS = static_cast <quint8> (list.value(1).toInt() < 0xFE ? list.value(1).toInt() : 0xFE);
            payload.time = qToLittleEndian <quint16> (list.value(2).toInt());

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

QByteArray Actions::ColorXY::request(const QVariant &data)
{
    zclHeaderStruct header;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = 0x07;

    switch (data.type())
    {
        case QVariant::List:
        {
            colorXYStruct payload;
            QList <QVariant> list = data.toList();
            double colorX = list.value(0).toDouble() * 0xFFFF, colorY = list.value(1).toDouble() * 0xFFFF;

            payload.colorX = qToLittleEndian <quint16> (colorX < 0xFEFF ? colorX : 0xFEFF);
            payload.colorY = qToLittleEndian <quint16> (colorY < 0xFEFF ? colorY : 0xFEFF);
            payload.time = qToLittleEndian <quint16> (list.value(2).toInt());

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

QByteArray Actions::ColorTemperature::request(const QVariant &data)
{
    zclHeaderStruct header;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = 0x0A;

    switch (data.type())
    {
        case QVariant::LongLong:
        {
            colorTemperatureStruct payload;

            payload.temperature = qToLittleEndian <quint16> (data.toInt() < 0xFEFF ? data.toInt() : 0xFEFF);
            payload.time = 0;

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::List:
        {
            colorTemperatureStruct payload;
            QList <QVariant> list = data.toList();

            payload.temperature = qToLittleEndian <quint16> (list.value(0).toInt() < 0xFEFF ? list.value(0).toInt() : 0xFEFF);
            payload.time = qToLittleEndian <quint16> (list.value(1).toInt());

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

QByteArray ActionsPTVO::ChangePattern::request(const QVariant &data)
{
    zclHeaderStruct header;
    QString status = data.toString();

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = status == "toggle" ? 0x02 : status == "on" ? 0x01 : 0x00;

    return QByteArray(reinterpret_cast <char*> (&header), sizeof(header));
}

QByteArray ActionsPTVO::Pattern::request(const QVariant &data)
{
    float value = data.toFloat();
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsLUMI::Sensitivity::request(const QVariant &data)
{
    QList <QString> list = {"low", "medium", "high"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    value += 1;
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsLUMI::Mode::request(const QVariant &data)
{
    QList <QString> list = {"undirected", "directed"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsLUMI::Distance::request(const QVariant &data)
{
    QList <QString> list = {"far", "middle", "near"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsLUMI::ResetPresence::request(const QVariant &data)
{
    quint8 value = 1; // TODO: check this

    if (!data.toBool())
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsTUYA::Request::makeRequest(quint8 transactionId, quint8 dataPoint, quint8 dataType, void *data)
{
    zclHeaderStruct zclHeader;
    tuyaHeaderStruct tuyaHeader;

    zclHeader.frameControl = FC_CLUSTER_SPECIFIC;
    zclHeader.transactionId = transactionId;
    zclHeader.commandId = 0x00;

    tuyaHeader.status = 0x00;
    tuyaHeader.transactionId = transactionId;
    tuyaHeader.dataPoint = dataPoint;
    tuyaHeader.dataType = dataType;
    tuyaHeader.function = 0x00;

    switch (tuyaHeader.dataType)
    {
        case 0x01:
        case 0x04:
            tuyaHeader.length = 1;
            break;

        case 0x02:
            tuyaHeader.length = 4;
            break;

        default:
            return QByteArray();
    }

    return QByteArray(reinterpret_cast <char*> (&zclHeader), sizeof(zclHeader)).append(reinterpret_cast <char*> (&tuyaHeader), sizeof(tuyaHeader)).append(reinterpret_cast <char*> (data), tuyaHeader.length);
}

QByteArray ActionsTUYA::Volume::request(const QVariant &data)
{
    QList <QString> list = {"low", "medium", "high"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return makeRequest(m_transactionId++, 0x05, 0x04, &value);
}

QByteArray ActionsTUYA::Duration::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toInt());

    if (value > 1800)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x07, 0x02, &value);
}

QByteArray ActionsTUYA::Alarm::request(const QVariant &data)
{
    quint8 value = data.toBool() ? 0x01 : 0x00;
    return makeRequest(m_transactionId++, 0x0D, 0x01, &value);
}

QByteArray ActionsTUYA::Melody::request(const QVariant &data)
{
    quint8 value = static_cast <quint8> (data.toInt());

    if (value < 1 || value > 18)
        return QByteArray();

    return makeRequest(m_transactionId++, 0x15, 0x04, &value);
}

QByteArray ActionsTUYA::Sensitivity::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toInt());

    if (value > 9)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x02, 0x02, &value);
}

QByteArray ActionsTUYA::DistanceMin::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toDouble() * 100);

    if (value > 950)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x03, 0x02, &value);
}

QByteArray ActionsTUYA::DistanceMax::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toDouble() * 100);

    if (value > 950)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x04, 0x02, &value);
}

QByteArray ActionsTUYA::DetectionDelay::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toInt());

    if (value < 1 || value > 10)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x65, 0x02, &value);
}

QByteArray ActionsTUYA::PowerOnStatus::request(const QVariant &data)
{
    QList <QString> list = {"off", "on", "previous"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsPerenio::PowerOnStatus::request(const QVariant &data)
{
    QList <QString> list = {"off", "on", "previous"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsPerenio::ResetAlarms::request(const QVariant &data)
{
    if (!data.toBool())
        return QByteArray();

    return writeAttributeRequest(QByteArray(1, 0x00));
}

QByteArray ActionsPerenio::AlarmVoltageMin::request(const QVariant &data)
{
    quint16 value = static_cast <quint16> (data.toInt());
    value = qToLittleEndian(value);
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsPerenio::AlarmVoltageMax::request(const QVariant &data)
{
    quint16 value = static_cast <quint16> (data.toInt());
    value = qToLittleEndian(value);
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsPerenio::AlarmPowerMax::request(const QVariant &data)
{
    quint16 value = static_cast <quint16> (data.toInt());
    value = qToLittleEndian(value);
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsPerenio::AlarmEnergyLimit::request(const QVariant &data)
{
    quint16 value = static_cast <quint16> (data.toInt());
    value = qToLittleEndian(value);
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}
