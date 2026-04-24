#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QVector>

#define SETTING_FNAME "PanelFinder2SettingFile.ini"

struct PanelSettingItem
{
    QString ipv4addr;
    QString uname;
    QString password;
};

class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int windowWidth  READ windowWidth  CONSTANT)
    Q_PROPERTY(int windowHeight READ windowHeight CONSTANT)
public:
    explicit AppSettings(QObject *parent = nullptr);

    int windowWidth()  const { return mWindowWidth; }
    int windowHeight() const { return mWindowHeight; }
    Q_INVOKABLE void saveWindowSize(int w, int h);

    void saveCredentials(const QString &ipv4addr, const QString &user, const QString &password);
    qint16 findCredentials(const QString &ipv4addr) const;
    const PanelSettingItem &credentialAt(qint16 idx) const;
    const PanelSettingItem &defaultCredential() const { return mDefault; }

private:
    void initFile();
    void loadAll();
    void flushWindowSize();

    int mWindowWidth  = 520;
    int mWindowHeight = 300;

    PanelSettingItem          mDefault;
    QVector<PanelSettingItem> mCredentials;
    QTimer                    mSaveTimer;
};

#endif // APPSETTINGS_H
