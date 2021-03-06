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
 
#include "resultform.h"
#include "ui_resultform.h"

#include <QTextCodec>
#include <QMimeDatabase>
#include <QSvgRenderer>
#include <QPainter>

ResultForm::ResultForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResultForm), m_request(0), m_response(0)
{
    ui->setupUi(this);

    connect(ui->requestRadio, SIGNAL(toggled(bool)), this, SLOT(renderData()));
    connect(ui->responseRadio, SIGNAL(toggled(bool)), this, SLOT(renderData()));

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(renderData()));

    connect(ui->encodingCBox, SIGNAL(currentTextChanged(QString)),
            this, SLOT(encodingChanged(QString)));
}

void ResultForm::encodingChanged(const QString &encoding)
{
    QTextCodec *codec = QTextCodec::codecForName(encoding.toLatin1());
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());

    if (codec) {
        if (comboBox)
            comboBox->setStyleSheet("");

        if (ui->requestRadio->isChecked()) {
            ui->bodyText->setPlainText(codec->toUnicode(m_request->body));
        } else if (ui->responseRadio->isChecked()) {
            ui->bodyText->setPlainText(codec->toUnicode(m_response->body));
        }
    } else if (comboBox)
       comboBox->setStyleSheet("QComboBox:editable:!on { background: #FF7777; color: white }");
}

void ResultForm::updateData(const HttpRequest *request, const HttpResponse *response,
                            const QStringList &encodings)
{
    m_request = request;
    m_response = response;
    m_encodings = &encodings;

    renderData();
}

void ResultForm::renderData()
{
    const QList<QNetworkReply::RawHeaderPair> *headers = nullptr;
    const QList<QNetworkCookie> *cookies = nullptr;
    const QByteArray *body = nullptr;

    if (ui->requestRadio->isChecked()) {
        ui->requestWidget->setHidden(false);

        headers = &m_request->headers;
        cookies = &m_request->cookies;
        body = &m_request->body;
    } else if (ui->responseRadio->isChecked()) {
        ui->requestWidget->setHidden(true);

        headers = &m_response->headers;
        cookies = &m_response->cookies;
        body = &m_response->body;
    }

    Q_ASSERT(cookies && headers && body);

    ui->methodLabel->setText(m_request->method);
    ui->urlEdit->setText(m_request->url);
    ui->urlEdit->setCursorPosition(0);

    ui->tabWidget->blockSignals(true);

    if (headers->isEmpty()) {
        ui->tabWidget->setTabEnabled(0, false);
        ui->headersText->clear();
    } else
        ui->tabWidget->setTabEnabled(0, true);

    if (ui->tabWidget->currentWidget() == ui->headersTab) {
        QString html = QStringLiteral("<table align='center'>");
        foreach (const QNetworkReply::RawHeaderPair &header, *headers) {
            html += QStringLiteral("<tr><td align='right' nowrap><u>%1</u>:</td><td>%2</td></tr>").
                    arg(QString::fromLatin1(header.first)).arg(QUrl::fromPercentEncoding(header.second));
        }
        html += QLatin1String("</table>");
        ui->headersText->setHtml(html);
    }

    if (cookies->isEmpty()) {
        ui->tabWidget->setTabEnabled(1, false);
        ui->cookiesText->clear();
    } else
        ui->tabWidget->setTabEnabled(1, true);

    if (ui->tabWidget->currentWidget() == ui->cookiesTab) {
        QString html = "<table align='center'>";
        foreach (const QNetworkCookie &cookie, *cookies) {
            QString name = QString::fromLatin1(cookie.name());
            QByteArray value = cookie.toRawForm().mid(name.length() + 1);
            html += QStringLiteral("<tr><td align='right' nowrap><u>%1</u>:</td><td>%2</td></tr>").
                    arg(name).arg(QUrl::fromPercentEncoding(value));
        }
        html += QStringLiteral("</table>");
        ui->cookiesText->setHtml(html);
    }

    if (body->isEmpty()) {
        ui->tabWidget->setTabEnabled(2, false);
    } else
        ui->tabWidget->setTabEnabled(2, true);

    if (ui->tabWidget->currentWidget() == ui->bodyTab) {
        QMimeDatabase mimeDatabase;
        QByteArray encoding;

        foreach (const QNetworkReply::RawHeaderPair &header, *headers) {
            const QString &headerName = QString::fromLatin1(header.first).toLower();
            const QString &headerValue = QString::fromLatin1(header.second).toLower();

            if (headerName == QLatin1String("content-type")) {
                QString contentType = headerValue.left(headerValue.indexOf(";"));
                const QMimeType mimeType = mimeDatabase.mimeTypeForName(contentType);

                ui->stackedWidget->setCurrentWidget(ui->unknownDataPage);
                ui->encodingCBox->setHidden(true);

                if (contentType == QLatin1String("image/png")
                        || contentType == QLatin1String("image/jpeg")
                        || contentType == QLatin1String("image/gif")
                        || contentType == QLatin1String("image/bmp")) {
                    QPixmap preview;
                    if (preview.loadFromData(*body)) {
                        ui->stackedWidget->setCurrentWidget(ui->imageDataPage);
                        ui->imagePreviewLabel->setPixmap(preview, QPen(Qt::gray, 1));
                    }
                } else if (contentType == QLatin1String("image/svg+xml")) {
                    QSvgRenderer svg(*body);

                    if (svg.isValid()) {
                        QPixmap preview(svg.viewBox().width(), svg.viewBox().height());
                        preview.fill(Qt::transparent);

                        QPainter painter;
                        painter.begin(&preview);
                        svg.render(&painter);
                        painter.end();

                        ui->stackedWidget->setCurrentWidget(ui->imageDataPage);
                        ui->imagePreviewLabel->setPixmap(preview, QPen(Qt::gray, 1));
                    }
                } else if (mimeType.inherits(QStringLiteral("text/plain"))
                      || contentType == QLatin1String("application/x-www-form-urlencoded")
                      || contentType == QLatin1String("multipart/form-data")) {
                      ui->stackedWidget->setCurrentWidget(ui->textDataPage);
                      ui->encodingCBox->setHidden(false);

                      QRegExp charsetRx("charset\\=([^;]+)", Qt::CaseInsensitive);
                      if (charsetRx.indexIn(headerValue) != -1) {
                          QByteArray charset = charsetRx.cap(1).toLatin1();

                          QRegExp encodingRx(charset, Qt::CaseInsensitive, QRegExp::Wildcard);
                          int encodingIdx = m_encodings->indexOf(encodingRx);
                          if (encodingIdx != -1)
                              encoding = m_encodings->at(encodingIdx).toLatin1();
                          else// if (QTextCodec::codecForName(charset))
                              encoding = charset;
                      }
                  }

                ui->mimeType->setText(contentType);
                ui->mimeType->setCursorPosition(0);
            }
        }

        ui->encodingCBox->blockSignals(true);
        ui->encodingCBox->clear();
        ui->encodingCBox->insertItems(0, *m_encodings);
        ui->encodingCBox->blockSignals(false);

        if (!QTextCodec::codecForName(encoding))
            ui->encodingCBox->setCurrentText(QStringLiteral("ISO-8859-1"));
        else
            ui->encodingCBox->setCurrentText(encoding);
    }

    ui->tabWidget->blockSignals(false);
}

ResultForm::~ResultForm()
{
    delete ui;
}
