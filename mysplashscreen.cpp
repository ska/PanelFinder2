#include "mysplashscreen.h"

#include <QGuiApplication>
#include <QPainter>

MySplashScreen::MySplashScreen(const QPixmap &pixmap, Qt::WindowFlags f)
    : QSplashScreen(pixmap, f)
{
    setFocusPolicy(Qt::StrongFocus);
    setWindowFlag(Qt::WindowStaysOnTopHint);
}

void MySplashScreen::showEvent(QShowEvent *event)
{
    QSplashScreen::showEvent(event);

    Qt::KeyboardModifiers mods = QGuiApplication::keyboardModifiers();
    if (mods & Qt::ControlModifier)
        m_ctrlPressed = true;
    if (mods & Qt::AltModifier)
        m_altPressed = true;
}

void MySplashScreen::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier)
        m_ctrlPressed = true;
    if (event->modifiers() & Qt::AltModifier)
        m_altPressed = true;

    QSplashScreen::keyPressEvent(event);
}

bool MySplashScreen::ctrlPressed() const { return m_ctrlPressed; }
bool MySplashScreen::altPressed()  const { return m_altPressed;  }

void MySplashScreen::setProgress(int value)
{
    m_progress = qBound(0, value, 100);
    repaint();
}

void MySplashScreen::setStatusText(const QString &t)
{
    m_statusText = t;
    repaint();
}

void MySplashScreen::drawContents(QPainter *p)
{
    // --- area progress bar ---
    const int barWidth  = width() - 40;
    const int barHeight = 8;
    const int x = 20;
    const int y = height() - 40;

    p->setPen(Qt::NoPen);
    p->setBrush(QColor(60, 60, 60, 180));
    p->drawRoundedRect(x, y, barWidth, barHeight, 4, 4);

    int fillWidth = (barWidth * m_progress) / 100;
    p->setBrush(QColor(0, 180, 255)); // colore barra
    p->drawRoundedRect(x, y, fillWidth, barHeight, 4, 4);

    if (!m_statusText.isEmpty()) {
        p->setPen(Qt::darkGray);
        p->drawText(
            QRect(20, y - 22, width() - 40, 20),
            Qt::AlignCenter,
            m_statusText
            );
    }
}
