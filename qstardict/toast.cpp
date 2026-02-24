/*****************************************************************************
 * toast.cpp - QStarDict, a dictionary application for learning languages    *
 * Copyright (C) 2025-2026 Alexander Rodin                                   *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,   *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.               *
 *****************************************************************************/

#include "toast.h"

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QScrollBar>
#include <QTimer>

namespace QStarDict
{

Toast::Toast(QWidget *parent)
    : QTextBrowser(parent)
{
    setVisible(false);

    m_timer = new QTimer(this);
    m_closeTimer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Toast::onTimerTimeout);
    connect(m_closeTimer, &QTimer::timeout, this, &Toast::onCloseTimerTimeout);

    m_informationPlayer = createMediaPlayer(QUrl("qrc:/sounds/dialog-information.oga"));
    m_warningPlayer = createMediaPlayer(QUrl("qrc:/sounds/dialog-warning.oga"));
}

void Toast::hide()
{
    setGeometry(geometry().x(), -geometry().height(), geometry().width(), geometry().height());
}

void Toast::show(const QString &html, ToastType toastType)
{
    setText(
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "<style>body { background-color: #aaaaaa; }</style>\n"
        "</head>\n"
        "<body>\n"
        "<p align=\"center\">\n" +
        html +
        "</p>");
    int toastWidth = int(parentWidget()->width() * 0.8);
    setGeometry((parentWidget()->width() - toastWidth) / 2, -height(), toastWidth, 70);
    QTextBrowser::show();

    while (verticalScrollBar()->isVisible())
        setGeometry(geometry().x(), geometry().y(),
                geometry().width(), geometry().height() + 1);

    m_toastType = toastType;
    m_timer->start(5);
}

void Toast::mousePressEvent(QMouseEvent*)
{
    hide();
}

QMediaPlayer *Toast::createMediaPlayer(const QUrl &soundUrl)
{
    QAudioOutput *audioOutput = new QAudioOutput(this);
    QMediaPlayer *player = new QMediaPlayer(this);
    player->setAudioOutput(audioOutput);

    player->setSource(soundUrl);
    // For some reason the first play might be not successful. Thus it is performed on startup
    // to ensure that the the sound is actually played when the toast is shown.
    audioOutput->setVolume(0.0);
    player->play();

    return player;
}

void Toast::onTimerTimeout()
{
    setGeometry(geometry().x(), geometry().y() + 1,
            geometry().width(), geometry().height());
    if (geometry().y() >= 0)
    {
        m_timer->stop();

        switch (m_toastType)
        {
            case ToastType::Information:
                m_informationPlayer->audioOutput()->setVolume(1.0);
                m_informationPlayer->play();
                break;
            case ToastType::Warning:
                m_warningPlayer->audioOutput()->setVolume(1.0);
                m_warningPlayer->play();
                break;
        }

        m_closeTimer->start(5000);
    }
}

void Toast::onCloseTimerTimeout()
{
    hide();
    m_closeTimer->stop();
}

}
