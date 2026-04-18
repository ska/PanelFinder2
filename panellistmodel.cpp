#include "panellistmodel.h"
/*********************** PanelListModel *****************************/
/**
 * @brief PanelListModel::PanelListModel
 */
PanelListModel::PanelListModel()
{
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(updateOrRemovePanels()));
    mTimer->start(5000);

    manager = new QNetworkAccessManager(this);
    QObject::connect(manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
                     this, SLOT(onAuthenticationRequestSlot(QNetworkReply*,QAuthenticator*)) );
    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(replyFinished(QNetworkReply*)));

    if (!QFile(SETTING_FNAME).exists())
    {
        QSettings settings(SETTING_FNAME, QSettings::IniFormat);
        //settings.setIniCodec("UTF-8");

        settings.beginGroup("default");
        settings.setValue("user", "admin");
        settings.setValue("password", "admin");
        settings.endGroup();

        //settings.beginGroup("192.168.1.*");
        //settings.setValue("user", "admin");
        //settings.setValue("passvord", "AdminXX");
        //settings.endGroup();

        settings.beginGroup("192.168.1.155");
        settings.setValue("user", "admin");
        settings.setValue("password", "Admin123@");
        settings.endGroup();

        settings.sync();
    }


    QSettings settings(SETTING_FNAME, QSettings::IniFormat);
    QStringList groups = settings.childGroups();

    foreach (QString settGroupName, groups)
    {
        qDebug() << "settGroupName: " << settGroupName;
        PanelSettingItem tmp;

        settings.beginGroup(settGroupName);
        if( "default" == settGroupName)
        {
            mPanelSettDefault.ipv4addr = settGroupName;
            mPanelSettDefault.uname    = settings.value("user").toString();
            mPanelSettDefault.password = settings.value("password").toString();
            settings.endGroup();
            continue;
        }
        tmp.ipv4addr = settGroupName;
        tmp.uname    = settings.value("user").toString();
        tmp.password = settings.value("password").toString();

        settings.endGroup();
        mPanelSettList.append(tmp);
    }
}

/**
 * @brief PanelListModel::~PanelListModel
 */
PanelListModel::~PanelListModel()
{
}

/**
 * @brief PanelListModel::updateOrRemovePanels
 */
void PanelListModel::updateOrRemovePanels()
{
    if(mTimer->interval() < 10000)
        mTimer->setInterval(10000);

    quint64 tmp = QDateTime::currentSecsSinceEpoch();
    for(quint16 i=0; i<mList.size();i++)
    {
        /*Remome if not present for more than 15sec */
        if( tmp - mList.at(i).foundEpoc > 15 )
        {
            qDebug() << "Remove panel: " << mList.at(i).macaddr;
            beginRemoveRows(QModelIndex(), i, i);
            mList.remove(i);
            endRemoveRows();
            i--;
            emit listChanged();
        } else {
            /* Update panel infos */
            //https://192.168.1.194/rest/api/v1?cache=true
            QString urls;
            urls = REQ_PROTO;
            urls.append( mList.at(i).ipv4addr );
            urls.append( REQ_PORT );
            urls.append( "/rest/api/v1?cache=true&js=true&js_var=_global_data" );
            QUrl url(urls);
            /*
             * SSL SELF SIGNED IGNORE */
            QSslConfiguration conf = request.sslConfiguration();
            conf.setPeerVerifyMode(QSslSocket::VerifyNone);
            request.setSslConfiguration(conf);
            /*
             * SET url AND REQUEST */
            request.setUrl( url );
            manager->get(request);
        }
    }
}

/**
 * @brief PanelListModel::toCidr
 * @param ipv4netmask
 * @return
 */
quint8 PanelListModel::toCidr(const QString ipv4netmask) const {
    QStringList nm = ipv4netmask.split('.');
    quint8 netmask_cidr = 0;

    for(int i=0; i<nm.size() && i<4; i++)
    {
        switch(nm.at(i).toInt())
        {
        case 0x80:
            netmask_cidr+=1;
            break;

        case 0xC0:
            netmask_cidr+=2;
            break;

        case 0xE0:
            netmask_cidr+=3;
            break;

        case 0xF0:
            netmask_cidr+=4;
            break;

        case 0xF8:
            netmask_cidr+=5;
            break;

        case 0xFC:
            netmask_cidr+=6;
            break;

        case 0xFE:
            netmask_cidr+=7;
            break;

        case 0xFF:
            netmask_cidr+=8;
            break;

        default:
            return netmask_cidr;
        }
    }
    return netmask_cidr;
}

/**
 * @brief PanelListModel::clearList
 */
void PanelListModel::clearList()
{
    quint16 size = mList.size();
    for(quint16 i=0; i<size;i++)
    {
        beginRemoveRows(QModelIndex(), size-i-1, size-i-1);
        mList.remove(size-i-1);
        endRemoveRows();
    }
    emit listChanged();
}

/**
 * @brief PanelListModel::addData
 * @param unit
 */
void PanelListModel::addData(const PanelItem &unit)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mList.append(unit);
    endInsertRows();
    this->sort(0, Qt::DescendingOrder);
    emit listChanged();
}

/**
 * @brief PanelListModel::insertData
 * @param unit
 */
void PanelListModel::insertData(const PanelItem &unit)
{
    quint16 i;
#ifdef QT_DEBUG
    qDebug() << "Try to insert data: " << unit.macaddr << " --> " << unit.foundEpoc;
#endif
    for(i=0; i<mList.size();i++)
    {
        if( (mList.at(i).macaddr == unit.macaddr) )
        {
            //qDebug() << "Found at: " << i << "/" << mList.size();
            mList[i].foundEpoc = unit.foundEpoc;
            if(     mList.at(i).hostname != unit.hostname ||
                mList.at(i).machine != unit.machine ||
                mList.at(i).ipv4addr != unit.ipv4addr ||
                mList.at(i).ipv4netmask != unit.ipv4netmask
                )
            {
                if(unit.hostname == "")
                {
                    PanelItem test;
                    test = unit;
                    test.hostname = mList.at(i).hostname;
                    beginRemoveRows(QModelIndex(), i, i);
                    mList.remove(i);
                    endRemoveRows();
                    this->addData(test);
                } else {
                    beginRemoveRows(QModelIndex(), i, i);
                    mList.remove(i);
                    endRemoveRows();
                    this->addData(unit);
                }
            }
            break;
        }
    }

    if(i==mList.size())
    {
        //qDebug() << "Not found: " << i << "/" << mList.size();
        this->addData(unit);
        //qDebug() << "New Size: " << mList.size();
    }
}

/**
 * @brief PanelListModel::rowCount
 * @param parent
 * @return
 */
int PanelListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mList.count();
}

/**
 * @brief PanelListModel::data
 * @param index
 * @param role
 * @return
 */
QVariant PanelListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= mList.count())
        return QVariant();

    const PanelItem item = mList.at(index.row());
    switch (role) {
    case HostnameRole:
        return QVariant(item.hostname);
    case MachineRole:
        return QVariant(item.machine);
    case MacaddressRole:
        return QVariant(item.macaddr);
    case Ipv4addrRole:
        return QVariant(item.ipv4addr);
    case Ipv4netmaskRole:
        return QVariant(this->toCidr(item.ipv4netmask));
    case MainOsVersionRole:
        return QVariant(item.mainosVersion);
    case ConfigOsVersionRole:
        return QVariant(item.configosVersion);
    case SerialNoRole:
        return QVariant(item.serialNo);
    }

    return QVariant();

}

/**
 * @brief PanelListModel::roleNames
 * @return
 */
QHash<int, QByteArray> PanelListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    //roles[HostnameRole] = "name";
    roles[HostnameRole]         = "hostname";
    roles[MachineRole]          = "machine";
    roles[MacaddressRole]       = "macaddr";
    roles[Ipv4addrRole]         = "ipv4addr";
    roles[Ipv4netmaskRole]      = "ipv4netmask";
    roles[MainOsVersionRole]    = "mainosVer";
    roles[ConfigOsVersionRole]  = "configosVer";
    roles[SerialNoRole]         = "serialNo";
    return roles;
}

/* Web Req Authentication slots
 * * PanelListModel::onAuthenticationRequestSlot()
 ********************************************************************/
void PanelListModel::onAuthenticationRequestSlot(QNetworkReply *reply, QAuthenticator *aAuthenticator)
{
    qDebug() << "PanelListModel::onAuthenticationRequestSlot";
    QString replyIP;
    static const QRegularExpression rx("^((http[s]?|ftp):\\/)?\\/?([^:\\/\\s]+)((\\/\\w+)*\\/)([\\w\\-\\.]+[^#?\\s]+)(.*)?(#[\\w\\-]+)?$");

    QRegularExpressionMatch match = rx.match(reply->url().toString());
    if( match.hasMatch() )
    {
        replyIP = match.captured(3);
    } else {
        replyIP = "NO host";
        return;
    }
    qDebug() << "AA replyIP: " << replyIP;

    //aAuthenticator->setUser( "admin" );
    //aAuthenticator->setPassword( "admin" );
    qint16 panelFound = findInPanelSetting(replyIP);
    qDebug() << "panelFound: " << panelFound;
    if(panelFound >= 0)
    {
        qDebug() << "Use: " << mPanelSettList.at(panelFound).uname << " - " <<  mPanelSettList.at(panelFound).password;
        aAuthenticator->setUser( mPanelSettList.at(panelFound).uname );
        aAuthenticator->setPassword( mPanelSettList.at(panelFound).password );
    } else {
        qDebug() << replyIP << " not found in file " << SETTING_FNAME;
        qDebug() << "Use default: " << mPanelSettDefault.uname << " - " <<  mPanelSettDefault.password;
        aAuthenticator->setUser(  mPanelSettDefault.uname );
        aAuthenticator->setPassword( mPanelSettDefault.password );
    }
}

/**
 * @brief PanelListModel::findInPanelSetting
 * @param r
 * @return
 ********************************************************************/
qint16 PanelListModel::findInPanelSetting( const QString r)
{
    qint16 match = -1;
    for(quint16 index=0; index< mPanelSettList.length(); index++)
    {
        QString tmp = mPanelSettList.at(index).ipv4addr;
        if( r.indexOf( tmp ) >= 0 )
        {
            match = index;
            break;
        }
    }
    return match;
}

/**
 * @brief PanelListModel::replyFinished
 * @param reply
 ********************************************************************/
void PanelListModel::replyFinished(QNetworkReply *reply)
{
    QString replyIP;
    static const QRegularExpression rx("^((http[s]?|ftp):\\/)?\\/?([^:\\/\\s]+)((\\/\\w+)*\\/)([\\w\\-\\.]+[^#?\\s]+)(.*)?(#[\\w\\-]+)?$");

    QRegularExpressionMatch match = rx.match(reply->url().toString());
    if( match.hasMatch() )
    {
        replyIP = match.captured(3);
    } else {
        replyIP = "NO host";
        return;
    }

    if (reply->error()) {
        reply->deleteLater();
        return;
    }

    QString answer = reply->readAll();
    reply->deleteLater();
    QJsonObject object = QJsonDocument::fromJson(answer.toUtf8()).object();
    //qDebug() << object;
    jsonFindValue( replyIP, &object );
}

/**
 * @brief PanelListModel::rebootPanel
 * @param ipadr
 * @param rt
 ********************************************************************/
void PanelListModel::rebootPanel(QString ipadr, quint8 rt)
{
    /*
     * CREA URL */
    QString urls;
    QByteArray data("{\"action\":\"restart\",\"imageType\":\"");
    data.append( QString("%1").arg(rt, 0, 10).toLocal8Bit() );
    data.append("\"}");

    urls = "https://";
    urls.append( ipadr );
    urls.append( "/rest/api/v1/system" );
    QUrl url(urls);

    /*
     * SSL SELF SIGNED IGNORE */
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    /*
     * SET url AND REQUEST */
    request.setUrl( url );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    //manager->get(request);
    manager->post(request, data);
}

/**
 * @brief PanelListModel::rebootMainOsPanel
 * @param ipadr
 ********************************************************************/
void PanelListModel::rebootMainOsPanel(QString ipadr)
{
    rebootPanel(ipadr, 0);
}

/**
 * @brief PanelListModel::rebootConfigOsPanel
 * @param ipadr
 ********************************************************************/
void PanelListModel::rebootConfigOsPanel(QString ipadr)
{
    rebootPanel(ipadr, 1);
}

/**
 * @brief PanelListModel::jsonFindValue
 ********************************************************************/
void PanelListModel::jsonFindValue(QString ip, QJsonObject *jobj)
{
    QVector<QString> path;
    jsonFindValueHelper(ip, jobj, path);
}

void PanelListModel::jsonFindValueHelper(QString ip, QJsonObject *jobj, QVector<QString> &path)
{
    for (auto i = jobj->begin(); i != jobj->end(); ++i)
    {
        if (i.value().isObject())
        {
            path.append(i.key());
            QJsonObject inn = i.value().toObject();
            jsonFindValueHelper(ip, &inn, path);
            path.removeLast();
        }
        else if (i.value().isArray())
        {
            QJsonArray qja = i.value().toArray();
            for (int k = 0; k < qja.size(); k++)
            {
                if (qja.at(k).isObject())
                {
                    path.append(QString("%1[%2]").arg(i.key()).arg(k));
                    QJsonObject inn = qja.at(k).toObject();
                    jsonFindValueHelper(ip, &inn, path);
                    path.removeLast();
                }
            }
        }
        else
        {
            QString fullPath = QStringList(path.begin(), path.end()).join('.')
                               + (path.isEmpty() ? "" : ".") + i.key();
            switch (i.value().type())
            {
            case QJsonValue::Bool:
                jsonParseValue(ip, fullPath, QString::number(i.value().toBool()));
                break;
            case QJsonValue::String:
                jsonParseValue(ip, fullPath, i.value().toString());
                break;
            case QJsonValue::Double:
                jsonParseValue(ip, fullPath, QString::number(i.value().toInt()));
                break;
            default:
                break;
            }
        }
    }
}

/**
 * @brief PanelListModel::jsonParseValue
 * @param ip
 * @param jsonpath
 * @param jsonvalue
 ********************************************************************/
void PanelListModel::jsonParseValue(QString ip, QString jsonpath, QString jsonvalue )
{
    //qDebug() << ip << jsonpath << jsonvalue;

    for(uint16_t i=0; i<mList.size();i++)
    {
        if(mList.at(i).ipv4addr == ip)
        {
            PanelItem tmpp = mList.at(i);
            bool changed = false;

            if("management.mainos.version" == jsonpath && tmpp.mainosVersion != jsonvalue) {
                tmpp.mainosVersion = jsonvalue;
                changed = true;
            } else if("management.configos.version" == jsonpath && tmpp.configosVersion != jsonvalue) {
                tmpp.configosVersion = jsonvalue;
                changed = true;
            } else if("system.info.info.serialNo" == jsonpath && tmpp.serialNo != jsonvalue) {
                tmpp.serialNo = jsonvalue;
                changed = true;
            }

            if(changed) {
                beginRemoveRows(QModelIndex(), i, i);
                mList.remove(i);
                endRemoveRows();
                this->addData(tmpp);
            }
            break;
        }
    }
}


/*********************** PanelListModel *****************************/


/******************** FilterProxyModel **************************/
FilterProxyModel::FilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{    
#if QT_VERSION >= 0x050A00
    mRandomNum = (quint8) ( QRandomGenerator::global()->generate()  % 99);
#else
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    mRandomNum = (qrand() % 100);
#endif
}
FilterProxyModel::~FilterProxyModel()
{
}
void FilterProxyModel::setFilterString(QString string)
{
    this->setFilterCaseSensitivity(Qt::CaseInsensitive);
    this->setFilterFixedString(string);
}
void FilterProxyModel::setClipboard(QClipboard *clipboard)
{
    mclipboard = clipboard;
}
void FilterProxyModel::copyIpToClipboard(QString ipadr)
{
    mclipboard->setText(ipadr);
}
QString FilterProxyModel::getVersion()
{
    return SW_VER;
}
QString FilterProxyModel::getName()
{
    return SW_NAME;
}


void FilterProxyModel::GetEasterSunday( quint16 wYear, quint16 &wMonth, quint16 &wDay )
{
    // calculate easter sunday
    // [in]  wYear  - 4-digit year to calculate (but not before 1583)
    // [out] wMonth - month of easter sunday
    // [out] wDay   - day of easter sunday

    quint16 wCorrection = 0;

    if( wYear < 1700 )      wCorrection = 4;
    else if( wYear < 1800 ) wCorrection = 5;
    else if( wYear < 1900 ) wCorrection = 6;
    else if( wYear < 2100 ) wCorrection = 0;
    else if( wYear < 2200 ) wCorrection = 1;
    else if( wYear < 2300 ) wCorrection = 2;
    else if( wYear < 2500 ) wCorrection = 3;

    wDay = (19 * (wYear % 19) + 24) % 30;
    wDay = 22 + wDay + ((2 * (wYear % 4) + 4 * (wYear % 7) + 6 * wDay + 5 + wCorrection) % 7);

    // jump to next month
    if( wDay > 31 )
    {
        wMonth = 4;
        wDay -= 31;
    }
    else
    {
        wMonth = 3;
    }
}


quint8 FilterProxyModel::getRandomNum() const
{
    quint16 eM, eD;
    FilterProxyModel::GetEasterSunday(QDate::currentDate().year(), eM, eD);
    QDate dEasterDate(QDate::currentDate().year(), eM, eD);
    QDate dNow(QDate::currentDate());
    qint16 dayTo = dNow.daysTo(dEasterDate);
    if(dayTo >= -1 && dayTo <= 2)
    {
        return 100;
    }

    return mRandomNum;
}
/******************** FilterProxyModel **************************/
