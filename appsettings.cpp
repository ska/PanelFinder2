#include "appsettings.h"
#include <QFile>
#include <QDebug>

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
{
    mSaveTimer.setSingleShot(true);
    mSaveTimer.setInterval(500);
    connect(&mSaveTimer, &QTimer::timeout, this, &AppSettings::flushWindowSize);

    initFile();
    loadAll();
}

void AppSettings::initFile()
{
    if (QFile(SETTING_FNAME).exists())
        return;

    QSettings s(SETTING_FNAME, QSettings::IniFormat);

    s.beginGroup("GUI");
    s.setValue("width",  mWindowWidth);
    s.setValue("height", mWindowHeight);
    s.endGroup();

    s.beginGroup("default");
    s.setValue("user",     "admin");
    s.setValue("password", "admin");
    s.endGroup();

    // Example entry — edit directly in the generated INI file
    s.beginGroup("192.168.1.155");
    s.setValue("user",     "admin");
    s.setValue("password", "Admin123@");
    s.endGroup();

    s.sync();
}

void AppSettings::loadAll()
{
    QSettings s(SETTING_FNAME, QSettings::IniFormat);

    s.beginGroup("GUI");
    mWindowWidth  = s.value("width",  520).toInt();
    mWindowHeight = s.value("height", 300).toInt();
    s.endGroup();

    const QStringList groups = s.childGroups();
    for (const QString &group : groups)
    {
        if (group == "GUI")
            continue;

        s.beginGroup(group);
        if (group == "default")
        {
            mDefault.ipv4addr = group;
            mDefault.uname    = s.value("user").toString();
            mDefault.password = s.value("password").toString();
        }
        else
        {
            PanelSettingItem item;
            item.ipv4addr = group;
            item.uname    = s.value("user").toString();
            item.password = s.value("password").toString();
            mCredentials.append(item);
        }
        s.endGroup();
        qDebug() << "AppSettings: loaded group:" << group;
    }
}

void AppSettings::saveWindowSize(int w, int h)
{
    mWindowWidth  = w;
    mWindowHeight = h;
    mSaveTimer.start();
}

void AppSettings::flushWindowSize()
{
    QSettings s(SETTING_FNAME, QSettings::IniFormat);
    s.beginGroup("GUI");
    s.setValue("width",  mWindowWidth);
    s.setValue("height", mWindowHeight);
    s.endGroup();
    s.sync();
}

void AppSettings::saveCredentials(const QString &ipv4addr, const QString &user, const QString &password)
{
    QSettings s(SETTING_FNAME, QSettings::IniFormat);
    s.beginGroup(ipv4addr);
    s.setValue("user",     user);
    s.setValue("password", password);
    s.endGroup();
    s.sync();

    qint16 idx = findCredentials(ipv4addr);
    if (idx >= 0)
    {
        mCredentials[idx].uname    = user;
        mCredentials[idx].password = password;
    }
    else
    {
        PanelSettingItem item;
        item.ipv4addr = ipv4addr;
        item.uname    = user;
        item.password = password;
        mCredentials.append(item);
    }
}

qint16 AppSettings::findCredentials(const QString &ipv4addr) const
{
    for (qint16 i = 0; i < mCredentials.size(); i++)
    {
        if (ipv4addr.indexOf(mCredentials.at(i).ipv4addr) >= 0)
            return i;
    }
    return -1;
}

const PanelSettingItem &AppSettings::credentialAt(qint16 idx) const
{
    return mCredentials.at(idx);
}
