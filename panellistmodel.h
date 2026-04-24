#ifndef CLISTMODEL_H
#define CLISTMODEL_H

#include <QSortFilterProxyModel>
#include <QNetworkAccessManager>
#include <QAbstractListModel>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QClipboard>
#include <QDateTime>
#include <QVector>
#include <QTimer>
#include <QDebug>
#if QT_VERSION >= 0x050A00
#include <QRandomGenerator>
#endif
#include "common.h"
#include "appsettings.h"

#define REQ_PROTO   "https://"
#define REQ_PORT    ""

struct PanelItem
{
    QString hostname;
    QString machine;
    QString macaddr;
    QString ipv4addr;
    QString ipv4netmask;
    QString mainosVersion;
    QString configosVersion;
    QString serialNo;
    qint64  foundEpoc;
};

enum Roles {
    HostnameRole = Qt::UserRole + 1,
    MachineRole,
    MacaddressRole,
    Ipv4addrRole,
    Ipv4netmaskRole,
    MainOsVersionRole,
    ConfigOsVersionRole,
    SerialNoRole,
};

//List Model
class PanelListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit PanelListModel(AppSettings *settings);
    ~PanelListModel();
    void addData(const PanelItem &unit);
    void insertData(const PanelItem &unit);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    void clearList();

    Q_INVOKABLE void rebootMainOsPanel(QString ipadr);
    Q_INVOKABLE void rebootConfigOsPanel(QString ipadr);
    Q_INVOKABLE void savePanelCredentials(const QString &ipv4addr, const QString &password, const QString &user = "admin");

protected:
    QHash<int, QByteArray> roleNames() const;

public slots:
    void updateOrRemovePanels();

private slots:
    void onAuthenticationRequestSlot(QNetworkReply *aReply, QAuthenticator *aAuthenticator);
    void replyFinished(QNetworkReply *reply);

signals:
    void listChanged();

private:
    AppSettings           *mSettings;
    QNetworkAccessManager *manager;
    QVector<PanelItem>     mList;
    QTimer                *mTimer;

    quint8 toCidr(const QString ipv4netmask) const;
    void jsonFindValue(QString ip, QJsonObject *jobj);
    void jsonFindValueHelper(QString ip, QJsonObject *jobj, QVector<QString> &path);
    void jsonParseValue(QString ip, QString jsonpath, QString jsonvalue);
    void rebootPanel(QString ipadr, quint8 rt);
};


//Filter proxy model
class FilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    FilterProxyModel(QObject* parent = 0);
    ~FilterProxyModel();
    void setClipboard(QClipboard *clipboard);
    static void GetEasterSunday(quint16 wYear, quint16 &wMonth, quint16 &wDay);

    Q_INVOKABLE void setFilterString(QString string);
    Q_INVOKABLE void copyIpToClipboard(QString ipadr);
    Q_INVOKABLE QString getVersion();
    Q_INVOKABLE QString getName();
    Q_INVOKABLE quint8 getRandomNum() const;

    QClipboard *mclipboard;
    quint8 mRandomNum;

private:
};


#endif // CLISTMODEL_H
