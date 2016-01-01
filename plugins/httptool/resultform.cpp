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

const QStringList ResultForm::encodings
{
    "IBM866",
    "ISO-8859-1",
    "ISO-8859-2",
    "ISO-8859-3",
    "ISO-8859-4",
    "ISO-8859-5",
    "ISO-8859-6",
    "ISO-8859-7",
    "ISO-8859-8",
    "ISO-8859-9",
    "ISO-8859-10",
    "ISO-8859-11",
    "ISO-8859-13",
    "ISO-8859-14",
    "ISO-8859-15",
    "ISO-8859-16",
    "Windows-1250",
    "Windows-1251",
    "Windows-1252",
    "Windows-1253",
    "Windows-1254",
    "Windows-1255",
    "Windows-1256",
    "Windows-1257",
    "Windows-1258",
    "KOI8-R",
    "KOI8-U",
    "UTF-8",
    "UTF-16"
};

ResultForm::ResultForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResultForm), m_request(0), m_response(0)
{
    ui->setupUi(this);

    connect(ui->requestRadio, SIGNAL(toggled(bool)), this, SLOT(renderData()));
    connect(ui->responseRadio, SIGNAL(toggled(bool)), this, SLOT(renderData()));

    connect(ui->requestTabWidget, SIGNAL(currentChanged(int)), this, SLOT(renderData()));
    connect(ui->responseTabWidget, SIGNAL(currentChanged(int)), this, SLOT(renderData()));

    connect(ui->requestEncodingCBox, SIGNAL(currentTextChanged(QString)),
            this, SLOT(requestEncodingChanged(QString)));

}

void ResultForm::requestEncodingChanged(const QString &encoding)
{
    QTextCodec *codec = QTextCodec::codecForName(encoding.isEmpty() ? "Latin1" : encoding.toLatin1());
    if (codec) {
        ui->requestEncodingCBox->setStyleSheet("");
        ui->requestBodyText->setPlainText(codec->toUnicode(m_request->body));
    } else
        ui->requestEncodingCBox->setStyleSheet("QComboBox:editable:!on { background: #FF7777; color: white }");
}

void ResultForm::responseEncodingChanged(const QString &encoding)
{
    QTextCodec *codec = QTextCodec::codecForName(encoding.isEmpty() ? "Latin1" : encoding.toLatin1());
    if (codec) {
        ui->responseEncodingCBox->setStyleSheet("");
        ui->responseBodyText->setPlainText(codec->toUnicode(m_response->body));
    } else
        ui->responseEncodingCBox->setStyleSheet("QComboBox:editable:!on { background: #FF7777; color: white }");
}

void ResultForm::updateData(const HttpRequest *request, const HttpResponse *response)
{
    m_request = request;
    m_response = response;

    renderData();
}

void ResultForm::renderData()
{
    if (ui->requestRadio->isChecked()) {
        const QList<QNetworkReply::RawHeaderPair> *requestHeaders = &m_request->headers;
        const QList<QNetworkCookie> *requestCookies = &m_request->cookies;

        ui->methodLabel->setText(m_request->method);
        ui->urlEdit->setText(m_request->url);
        ui->urlEdit->setCursorPosition(0);
        ui->requestWidget->show();

        ui->stackedWidget->setCurrentWidget(ui->requestPage);

        ui->requestTabWidget->blockSignals(true);

        if (requestHeaders->isEmpty()) {
            ui->requestTabWidget->setTabEnabled(0, false);
            ui->requestHeadersText->clear();
        } else
            ui->requestTabWidget->setTabEnabled(0, true);

        if (ui->requestTabWidget->currentWidget() == ui->requestHeadersTab) {
            QString html = "<table align='center'>";
            foreach (QNetworkReply::RawHeaderPair header, *requestHeaders) {
                html += QString("<tr><td align='right' nowrap><u>%1</u>:</td><td>%2</td></tr>").
                        arg(QString(header.first)).arg(QString(header.second));
            }
            html += "</table>";
            ui->requestHeadersText->setHtml(html);
        }

        if (requestCookies->isEmpty()) {
            ui->requestTabWidget->setTabEnabled(1, false);
            ui->requestCookiesText->clear();
        } else
            ui->requestTabWidget->setTabEnabled(1, true);

        if (ui->requestTabWidget->currentWidget() == ui->requestCookiesTab) {
            QString html = "<table align='center'>";
            foreach (QNetworkCookie cookie, *requestCookies) {
                QString name(cookie.name());
                QString value(cookie.toRawForm().mid(name.length() + 1));
                html += QString("<tr><td align='right' nowrap><u>%1</u>:</td><td>%2</td></tr>").
                        arg(name).arg(value);
            }
            html += "</table>";
            ui->requestCookiesText->setHtml(html);
        }

        if (m_request->body.isEmpty()) {
            ui->requestTabWidget->setTabEnabled(2, false);
            ui->requestBodyText->clear();
        } else
            ui->requestTabWidget->setTabEnabled(2, true);

        if (ui->requestTabWidget->currentWidget() == ui->requestBodyTab) {
            QString encoding;
            foreach (QNetworkReply::RawHeaderPair header, *requestHeaders) {
                if (QString(header.first).toLower() == "content-type"
                        && header.second.startsWith("text/")) {
                    QRegExp charsetRx("charset\\=([^;]+)", Qt::CaseInsensitive);
                    if (charsetRx.indexIn(header.second) != -1) {
                        QByteArray charset = charsetRx.cap(1).toLatin1();
                        qDebug() << "CAP: " << charset;

                        QRegExp encodingRx(charset, Qt::CaseInsensitive, QRegExp::Wildcard);
                        int encodingIdx = encodings.indexOf(encodingRx);
                        if (encodingIdx != -1)
                            encoding = encodings[encodingIdx];
                        else if (QTextCodec::codecForName(charset))
                            encoding = charset;
                    }
                }
            }

            disconnect(ui->requestEncodingCBox, SIGNAL(currentTextChanged(QString)),
                      this, SLOT(requestEncodingChanged(QString)));
            ui->requestEncodingCBox->clear();
            ui->requestEncodingCBox->insertItems(0, encodings);
            ui->requestEncodingCBox->setCurrentText(encoding);
            connect(ui->requestEncodingCBox, SIGNAL(currentTextChanged(QString)),
                    this, SLOT(requestEncodingChanged(QString)));

            requestEncodingChanged(encoding);
        }

        ui->requestTabWidget->blockSignals(false);
    } else if (ui->responseRadio->isChecked()) {
        const QList<QNetworkReply::RawHeaderPair> *responseHeaders = &m_response->headers;
        const QList<QNetworkCookie> *responseCookies = &m_response->cookies;

        ui->requestWidget->hide();
        ui->stackedWidget->setCurrentWidget(ui->responsePage);

        ui->responseTabWidget->blockSignals(true);

        if (responseHeaders->isEmpty()) {
            ui->responseTabWidget->setTabEnabled(0, false);
            ui->responseHeadersText->clear();
        } else
            ui->responseTabWidget->setTabEnabled(0, true);

        if (ui->responseTabWidget->currentWidget() == ui->responseHeadersTab) {
            QString html = "<table align='center'>";
            foreach (QNetworkReply::RawHeaderPair header, *responseHeaders) {
                html += QString("<tr><td align='right' nowrap><u>%1</u>:</td><td>%2</td></tr>").
                        arg(QString(header.first)).arg(QString(header.second));
            }

            html += "</table>";
            ui->responseHeadersText->setHtml(html);
        }

        if (responseCookies->isEmpty()) {
            ui->responseTabWidget->setTabEnabled(1, false);
            ui->responseCookiesText->clear();
        } else
            ui->responseTabWidget->setTabEnabled(1, true);

        if (ui->responseTabWidget->currentWidget() == ui->responseCookiesTab) {
            QString html = "<table align='center'>";
            foreach (QNetworkCookie cookie, *responseCookies) {
                QString name(cookie.name());
                QString value(cookie.toRawForm().mid(name.length() + 1));
                html += QString("<tr><td align='right' nowrap><u>%1</u>:</td><td>%2</td></tr>").
                        arg(name).arg(value);
            }
            html += "</table>";

            ui->responseCookiesText->setHtml(html);
        }

        if (m_response->body.isEmpty()) {
            ui->responseTabWidget->setTabEnabled(2, false);
            ui->responseBodyText->clear();
        } else
            ui->responseTabWidget->setTabEnabled(2, true);

        if (ui->responseTabWidget->currentWidget() == ui->responseBodyTab) {
            QString encoding;
            foreach (QNetworkReply::RawHeaderPair header, *responseHeaders) {
                if (QString(header.first).toLower() == "content-type"
                        && header.second.startsWith("text/")) {
                    QRegExp charsetRx("charset\\=([^;]+)", Qt::CaseInsensitive);
                    if (charsetRx.indexIn(header.second) != -1) {
                        QByteArray charset = charsetRx.cap(1).toLatin1();
                        qDebug() << "CAP: " << charset;

                        QRegExp encodingRx(charset, Qt::CaseInsensitive, QRegExp::Wildcard);
                        int encodingIdx = encodings.indexOf(encodingRx);
                        if (encodingIdx != -1)
                            encoding = encodings[encodingIdx];
                        else if (QTextCodec::codecForName(charset))
                            encoding = charset;
                    }
                }
            }

            disconnect(ui->responseEncodingCBox, SIGNAL(currentTextChanged(QString)),
                      this, SLOT(responseEncodingChanged(QString)));
            ui->responseEncodingCBox->clear();
            ui->responseEncodingCBox->insertItems(0, encodings);
            ui->responseEncodingCBox->setCurrentText(encoding);
            connect(ui->responseEncodingCBox, SIGNAL(currentTextChanged(QString)),
                    this, SLOT(responseEncodingChanged(QString)));

            responseEncodingChanged(encoding);
        }

        ui->responseTabWidget->blockSignals(false);
    }
}

ResultForm::~ResultForm()
{
    delete ui;
}
