#include "appsettings.h"
#include <QFile>
#include <QDebug>

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
{
    mSaveTimer.setSingleShot(true);
    mSaveTimer.setInterval(500);
    connect(&mSaveTimer, &QTimer::timeout, this, &AppSettings::flushWindowSize);

    if (!QFile(SETTING_FNAME).exists())
        initNewFiles();
    else
        loadAll();
}

// Called only when the main INI does not exist yet.
// Writes both files from scratch with default values.
void AppSettings::initNewFiles()
{
    mCredentialsFile  = "PanelFinder2Credentials.ini";
    mDefault.ipv4addr = "default";
    mDefault.uname    = "admin";
    mDefault.password = "admin";

    QSettings main(SETTING_FNAME, QSettings::IniFormat);
    main.beginGroup("GUI");
    main.setValue("width",         mWindowWidth);
    main.setValue("height",        mWindowHeight);
    main.setValue("reduceOnClose", mReduceOnClose);
    main.endGroup();
    main.beginGroup("default");
    main.setValue("user",     mDefault.uname);
    main.setValue("password", mDefault.password);
    main.endGroup();
    main.beginGroup("files");
    main.setValue("credentials", mCredentialsFile);
    main.endGroup();
    main.sync();

    // Example credential entry in the credentials file
    PanelSettingItem example{"192.168.1.155", "admin", "Admin123@"};
    mCredentials.append(example);

    QSettings cred(mCredentialsFile, QSettings::IniFormat);
    cred.beginGroup(example.ipv4addr);
    cred.setValue("user",     example.uname);
    cred.setValue("password", example.password);
    cred.endGroup();
    cred.sync();
}

void AppSettings::loadAll()
{
    QSettings main(SETTING_FNAME, QSettings::IniFormat);

    main.beginGroup("GUI");
    mWindowWidth   = main.value("width",         520).toInt();
    mWindowHeight  = main.value("height",        300).toInt();
    mReduceOnClose = main.value("reduceOnClose", true).toBool();
    main.endGroup();

    main.beginGroup("default");
    mDefault.ipv4addr = "default";
    mDefault.uname    = main.value("user",     "admin").toString();
    mDefault.password = main.value("password", "admin").toString();
    main.endGroup();

    const QStringList groups = main.childGroups();

    if (groups.contains("files")) {
        main.beginGroup("files");
        mCredentialsFile = main.value("credentials", "PanelFinder2Credentials.ini").toString();
        main.endGroup();
        loadCredentials();
    } else {
        // Old single-file format — migrate transparently
        migrateToSplitFiles(main, groups);
    }
}

void AppSettings::loadCredentials()
{
    QSettings cred(mCredentialsFile, QSettings::IniFormat);
    const QStringList groups = cred.childGroups();
    for (const QString &group : groups)
    {
        cred.beginGroup(group);
        PanelSettingItem item;
        item.ipv4addr = group;
        item.uname    = cred.value("user").toString();
        item.password = cred.value("password").toString();
        mCredentials.append(item);
        cred.endGroup();
        qDebug() << "AppSettings: loaded credential:" << group;
    }
}

// Reads IP groups from the old single file, writes them to the credentials file,
// then rewrites the main file adding [files] and removing the IP groups.
void AppSettings::migrateToSplitFiles(QSettings &main, const QStringList &groups)
{
    mCredentialsFile = "PanelFinder2Credentials.ini";

    for (const QString &group : groups)
    {
        if (group == "GUI" || group == "default")
            continue;
        main.beginGroup(group);
        PanelSettingItem item;
        item.ipv4addr = group;
        item.uname    = main.value("user").toString();
        item.password = main.value("password").toString();
        mCredentials.append(item);
        main.endGroup();
        qDebug() << "AppSettings: migrating credential group:" << group;
    }

    // Write credentials file
    QSettings cred(mCredentialsFile, QSettings::IniFormat);
    for (const PanelSettingItem &item : mCredentials)
    {
        cred.beginGroup(item.ipv4addr);
        cred.setValue("user",     item.uname);
        cred.setValue("password", item.password);
        cred.endGroup();
    }
    cred.sync();

    // Remove IP groups from main file and add [files] group.
    // main QSettings object is still open — close it first by letting it go
    // out of scope in loadAll(), then reopen for writing.
    // We capture what we need before returning.
    // (main is reused here since we're still in the same call frame)
    for (const PanelSettingItem &item : mCredentials)
    {
        main.beginGroup(item.ipv4addr);
        main.remove("");   // removes all keys in this group
        main.endGroup();
    }
    main.beginGroup("files");
    main.setValue("credentials", mCredentialsFile);
    main.endGroup();
    main.sync();

    qDebug() << "AppSettings: migration complete ->"  << mCredentialsFile;
}

void AppSettings::saveWindowSize(int w, int h)
{
    mWindowWidth  = w;
    mWindowHeight = h;
    mSaveTimer.start();
}

void AppSettings::flushWindowSize()
{
    QSettings main(SETTING_FNAME, QSettings::IniFormat);
    main.beginGroup("GUI");
    main.setValue("width",  mWindowWidth);
    main.setValue("height", mWindowHeight);
    main.endGroup();
    main.sync();
}

void AppSettings::setReduceOnClose(bool value)
{
    if (mReduceOnClose == value)
        return;
    mReduceOnClose = value;
    QSettings main(SETTING_FNAME, QSettings::IniFormat);
    main.beginGroup("GUI");
    main.setValue("reduceOnClose", mReduceOnClose);
    main.endGroup();
    main.sync();
    emit reduceOnCloseChanged();
}

void AppSettings::saveCredentials(const QString &ipv4addr, const QString &user, const QString &password)
{
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

    QSettings cred(mCredentialsFile, QSettings::IniFormat);
    cred.beginGroup(ipv4addr);
    cred.setValue("user",     user);
    cred.setValue("password", password);
    cred.endGroup();
    cred.sync();
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
