#ifndef CLISTMODEL_H
#define CLISTMODEL_H

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QVector>
#include <QDebug>
#include <QClipboard>
#include <QDateTime>
#include <QTimer>
#if QT_VERSION >= 0x050A00
#include <QRandomGenerator>
#endif


struct PanelItem
{
    QString hostname;
    QString machine;
    QString macaddr;
    QString ipv4addr;
    QString ipv4netmask;
    qint64  foundEpoc;
};

enum Roles {
    HostnameRole = Qt::UserRole + 1,
    MachineRole,
    MacaddressRole,
    Ipv4addrRole,
    Ipv4netmaskRole,
};

//List Model
class PanelListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    PanelListModel();
    ~PanelListModel();
    void addData(const PanelItem &unit);
    void insertData(const PanelItem &unit);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

protected:
    QHash<int, QByteArray> roleNames() const;

public slots:
    void removePanels();
signals:
    void listChanged();

private:
    quint8 toCidr(const QString ipv4netmask) const;
    QVector <PanelItem> mList;
};


//Filter proxy model
class FilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    FilterProxyModel(QObject* parent = 0);
    ~FilterProxyModel();
    void setClipboard(QClipboard *clipboard);

    Q_INVOKABLE void setFilterString(QString string);
    Q_INVOKABLE void copyIpToClipboard(QString ipadr);
    Q_INVOKABLE QString getVersion();
    Q_INVOKABLE quint8 getRandomNum() const;

    QClipboard *mclipboard;
    quint8 mRandomNum;
};


#endif // CLISTMODEL_H
