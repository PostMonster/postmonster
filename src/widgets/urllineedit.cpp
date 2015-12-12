/* PostMonster, universal HTTP automation tool
 * Copyright (C) 2015 by Paul Artsishevsky <polter.rnd@gmail.com>
 *
 * This file is part of PostMonster.
 *
 * PostMonster is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PostMonster is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PostMonster.  If not, see <http://www.gnu.org/licenses/>.
 */
 
/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "urllineedit.h"

//#include "searchlineedit.h"

#include <QtCore/QEvent>
#include <QtCore/QMimeData>

#include <QtWidgets/QApplication>
#include <QtWidgets/QCompleter>
#include <QtGui/QFocusEvent>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtGui/QDrag>
#include <QtGui/QPainter>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOptionFrameV2>

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QMenu>


#include <QtCore/QDebug>

ClearButton::ClearButton(QWidget *parent)
  : QAbstractButton(parent)
{
#ifndef QT_NO_CURSOR
    setCursor(Qt::ArrowCursor);
#endif // QT_NO_CURSOR
    setToolTip(tr("Clear"));
    setVisible(false);
    setFocusPolicy(Qt::NoFocus);
}

void ClearButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    int height = this->height();

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(isDown()
                     ? palette().color(QPalette::Dark)
                     : palette().color(QPalette::Mid));
    painter.setPen(painter.brush().color());
    int size = width();
    int offset = size / 5;
    int radius = size - offset * 2;
    painter.drawEllipse(offset, offset, radius, radius);

    painter.setPen(palette().color(QPalette::Base));
    int border = offset * 2;
    painter.drawLine(border, border, width() - border, height - border);
    painter.drawLine(border, height - border, width() - border, border);
}

void ClearButton::textChanged(const QString &text)
{
    setVisible(!text.isEmpty());
}

ExLineEdit::ExLineEdit(QWidget *parent)
    : QWidget(parent)
    , m_lineEdit(new QLineEdit(this))
    , m_clearButton(0)
{
    setFocusPolicy(m_lineEdit->focusPolicy());
    setAttribute(Qt::WA_InputMethodEnabled);
    setSizePolicy(m_lineEdit->sizePolicy());
    setBackgroundRole(m_lineEdit->backgroundRole());
    setMouseTracking(true);
    setAcceptDrops(true);
    setAttribute(Qt::WA_MacShowFocusRect, true);
    QPalette p = m_lineEdit->palette();
    setPalette(p);

    // line edit
    m_lineEdit->setFrame(false);
    m_lineEdit->setFocusProxy(this);
    m_lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    QPalette clearPalette = m_lineEdit->palette();
    clearPalette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    m_lineEdit->setPalette(clearPalette);

    // clearButton
    m_clearButton = new ClearButton(this);
    connect(m_clearButton, SIGNAL(clicked()),
            m_lineEdit, SLOT(clear()));
    connect(m_lineEdit, SIGNAL(textChanged(QString)),
            m_clearButton, SLOT(textChanged(QString)));
}

void ExLineEdit::resizeEvent(QResizeEvent *event)
{
    updateGeometries();
    QWidget::resizeEvent(event);
}

void ExLineEdit::updateGeometries()
{
    QStyleOptionFrameV2 panel;
    initStyleOption(&panel);
    QRect rect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);

    int clearButtonWidth = this->height();
    m_lineEdit->setGeometry(5, 0,
                            rect.width() - clearButtonWidth - 5,
                            this->height());

    m_clearButton->setGeometry(this->width() - clearButtonWidth, 0,
                               clearButtonWidth, this->height());
}

void ExLineEdit::initStyleOption(QStyleOptionFrameV2 *option) const
{
    option->initFrom(this);
    option->rect = contentsRect();
    option->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, option, this);
    option->midLineWidth = 0;
    option->state |= QStyle::State_Sunken;
    if (m_lineEdit->isReadOnly())
        option->state |= QStyle::State_ReadOnly;
#ifdef QT_KEYPAD_NAVIGATION
    if (hasEditFocus())
        option->state |= QStyle::State_HasEditFocus;
#endif
    option->features = QStyleOptionFrameV2::None;
}

QSize ExLineEdit::sizeHint() const
{
    m_lineEdit->setFrame(true);
    QSize size = m_lineEdit->sizeHint();
    m_lineEdit->setFrame(false);
    return size;
}

void ExLineEdit::focusInEvent(QFocusEvent *event)
{
    m_lineEdit->event(event);
    QWidget::focusInEvent(event);
}

void ExLineEdit::focusOutEvent(QFocusEvent *event)
{
    m_lineEdit->event(event);

    if (m_lineEdit->completer()) {
        connect(m_lineEdit->completer(), SIGNAL(activated(QString)),
                         m_lineEdit, SLOT(setText(QString)));
        connect(m_lineEdit->completer(), SIGNAL(highlighted(QString)),
                         m_lineEdit, SLOT(_q_completionHighlighted(QString)));
    }
    QWidget::focusOutEvent(event);
}

void ExLineEdit::keyPressEvent(QKeyEvent *event)
{
    m_lineEdit->event(event);
}

bool ExLineEdit::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
        return m_lineEdit->event(event);
    return QWidget::event(event);
}

void ExLineEdit::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOptionFrameV2 panel;
    initStyleOption(&panel);
    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);
}

QVariant ExLineEdit::inputMethodQuery(Qt::InputMethodQuery property) const
{
    return m_lineEdit->inputMethodQuery(property);
}

void ExLineEdit::inputMethodEvent(QInputMethodEvent *e)
{
    m_lineEdit->event(e);
}

UrlLineEdit::UrlLineEdit(QWidget *parent)
    : ExLineEdit(parent)
    , m_webView(0)
{
    m_defaultBaseColor = palette().color(QPalette::Base);
}

QString UrlLineEdit::text()
{
    return m_lineEdit->text();
}

void UrlLineEdit::setWebView(QWebView *webView)
{
    Q_ASSERT(!m_webView);
    m_webView = webView;
    connect(webView, SIGNAL(urlChanged(QUrl)),
        this, SLOT(webViewUrlChanged(QUrl)));
    connect(webView, SIGNAL(loadProgress(int)),
        this, SLOT(updateProgress(int)));
    connect(webView, SIGNAL(loadFinished(bool)),
            this, SLOT(clearProgress()));
    connect(m_lineEdit, SIGNAL(returnPressed()),
        this, SIGNAL(editingFinished()));
}

void UrlLineEdit::updateProgress(int progress)
{
    m_progress = progress;
    update();
}

void UrlLineEdit::clearProgress()
{
    m_progress = 0;
    update();
}

void UrlLineEdit::webViewUrlChanged(const QUrl &url)
{
    m_lineEdit->setText(QString::fromUtf8(url.toEncoded()));
    m_lineEdit->setCursorPosition(0);
}

QLinearGradient UrlLineEdit::generateGradient(const QColor &color) const
{
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, m_defaultBaseColor);
    gradient.setColorAt(0.15, color.lighter(120));
    gradient.setColorAt(0.5, color);
    gradient.setColorAt(0.85, color.lighter(120));
    gradient.setColorAt(1, m_defaultBaseColor);
    return gradient;
}

void UrlLineEdit::focusOutEvent(QFocusEvent *event)
{
    if (m_lineEdit->text().isEmpty() && m_webView)
        m_lineEdit->setText(QString::fromUtf8(m_webView->url().toEncoded()));
    ExLineEdit::focusOutEvent(event);
}

void UrlLineEdit::paintEvent(QPaintEvent *event)
{
    QPalette p = palette();
    if (m_webView && m_webView->url().scheme() == QLatin1String("https")) {
        QColor lightYellow(248, 248, 210);
        p.setBrush(QPalette::Base, generateGradient(lightYellow));
    } else {
        p.setBrush(QPalette::Base, m_defaultBaseColor);
    }
    setPalette(p);
    ExLineEdit::paintEvent(event);

    QPainter painter(this);
    QStyleOptionFrameV2 panel;
    initStyleOption(&panel);
    QRect backgroundRect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
    if (m_webView && !hasFocus()) {
        QColor loadingColor = QColor(116, 192, 250);
        painter.setBrush(generateGradient(loadingColor));
        painter.setPen(Qt::transparent);
        int mid = backgroundRect.width() / 100 * m_progress;
        QRect progressRect(backgroundRect.x(), backgroundRect.y(), mid, backgroundRect.height());
        painter.drawRect(progressRect);
    }
}
