#include "panellistmodel.h"

/*********************** PanelListModel *****************************/
PanelListModel::PanelListModel()
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(removePanels()));
    timer->start(10000);
}

PanelListModel::~PanelListModel()
{
}

void PanelListModel::removePanels()
{
    quint64 tmp = QDateTime::currentSecsSinceEpoch();
    for(quint16 i=0; i<mList.size();i++)
    {
        if( tmp - mList.at(i).foundEpoc > 15 )
        {
            qDebug() << "Remove panel: " << mList.at(i).macaddr;
            beginRemoveRows(QModelIndex(), i, i);
            mList.remove(i);
            endRemoveRows();
            emit listChanged();
        }
    }
}

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

void PanelListModel::addData(const PanelItem &unit)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mList.append(unit);
    endInsertRows();
    this->sort(0, Qt::DescendingOrder);
    emit listChanged();
}

void PanelListModel::insertData(const PanelItem &unit)
{
    quint16 i;
#ifdef QT_DEBUG
    qDebug() << "Try to insert data: " << unit.macaddr;
    qDebug() << "--> " << unit.foundEpoc;
#endif
    for(i=0; i<mList.size();i++)
    {
        if(mList.at(i).macaddr == unit.macaddr)
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

int PanelListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mList.count();
}

QVariant PanelListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= mList.count())
        return QVariant();
    /*
    const QString &name = mList[index.row()].hostname;
    if (role == HostnameRole)
        return name;
    return QVariant();
*/
    const PanelItem item = mList.at(index.row());
    //const TestItem item = mList[index.row()].hostname;
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
    }

    return QVariant();

}

QHash<int, QByteArray> PanelListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    //roles[HostnameRole] = "name";
    roles[HostnameRole]     = "hostname";
    roles[MachineRole]      = "machine";
    roles[MacaddressRole]   = "macaddr";
    roles[Ipv4addrRole]     = "ipv4addr";
    roles[Ipv4netmaskRole]  = "ipv4netmask";
    return roles;
}
/*********************** PanelListModel *****************************/


/******************** FilterProxyModel **************************/
FilterProxyModel::FilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{    
#if QT_VERSION >= 0x050A00
    mRandomNum = (quint8) ( QRandomGenerator::global()->generate()  % 100);
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
    return "v2.2";
}
quint8 FilterProxyModel::getRandomNum() const
{
    return mRandomNum;
}
/******************** FilterProxyModel **************************/
