#ifndef MYSPLASHSCREEN_H
#define MYSPLASHSCREEN_H
//#pragma once

#include <QSplashScreen>
#include <QKeyEvent>

class MySplashScreen : public QSplashScreen
{
    Q_OBJECT

public:
    explicit MySplashScreen(const QPixmap &pixmap,
                            Qt::WindowFlags f = Qt::WindowFlags());

    // tastiera
    bool ctrlPressed() const;
    bool altPressed() const;

    // progress
    void setProgress(int value);          // 0 - 100
    void setStatusText(const QString &t); // testo opzionale

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void drawContents(QPainter *painter) override;

private:
    bool m_ctrlPressed = false;
    bool m_altPressed  = false;

    int     m_progress = 0;
    QString m_statusText;
};
#endif
