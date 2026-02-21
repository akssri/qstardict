/*****************************************************************************
 * toast.h - QStarDict, a dictionary application for learning languages      *
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

#ifndef TOAST_H
#define TOAST_H

#include <QTextBrowser>

class QAudioOutput;
class QMediaPlayer;
class QTimer;

namespace QStarDict
{

class Toast: public QTextBrowser
{
    Q_OBJECT

    public:
        enum class ToastType
        {
            Information,
            Warning,
        };

        /**
         * The constructor. The parent widget must be provided.
         */
        Toast(QWidget *parent);

    public slots:
        void hide();
        void show(const QString &html, ToastType toastType);

    protected:
        void mousePressEvent(QMouseEvent*);

    private slots:
        void onTimerTimeout();
        void onCloseTimerTimeout();

    private:
        QMediaPlayer *m_informationPlayer;
        QMediaPlayer *m_warningPlayer;

        ToastType m_toastType;

        QTimer *m_timer;
        QTimer *m_closeTimer;

        QMediaPlayer *createMediaPlayer(const QUrl &soundUrl);
};

}

#endif // TOAST_H

