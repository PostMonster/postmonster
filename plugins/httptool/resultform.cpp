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
    connect(ui->responseEncodingCBox, SIGNAL(currentTextChanged(QString)),
            this, SLOT(responseEncodingChanged(QString)));
}

void ResultForm::requestEncodingChanged(const QString &encoding)
{
    QTextCodec *codec = QTextCodec::codecForName(encoding.toLocal8Bit());
    ui->requestBodyText->setPlainText(codec->toUnicode(m_request->body));
}

void ResultForm::responseEncodingChanged(const QString &encoding)
{
    QTextCodec *codec = QTextCodec::codecForName(encoding.toLocal8Bit());
    ui->responseBodyText->setPlainText(codec->toUnicode(m_response->body));
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
            foreach (QNetworkReply::RawHeaderPair header, *requestHeaders)
               html += QString("<tr><td align='right'><u>%1</u>:</td><td>%2</td></tr>").
                       arg(QString(header.first)).arg(QString(header.second));
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
                html += QString("<tr><td align='right'><u>%1</u>:</td><td>%2</td></tr>").
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

        if (ui->requestTabWidget->currentWidget() == ui->requestBodyTab)
            ui->requestBodyText->setPlainText(m_request->body);

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
            foreach (QNetworkReply::RawHeaderPair header, *responseHeaders)
               html += QString("<tr><td align='right'><u>%1</u>:</td><td>%2</td></tr>").
                       arg(QString(header.first)).arg(QString(header.second));
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
                html += QString("<tr><td align='right'><u>%1</u>:</td><td>%2</td></tr>").
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

        if (ui->responseTabWidget->currentWidget() == ui->responseBodyTab)
            ui->responseBodyText->setPlainText(m_response->body);

        ui->responseTabWidget->blockSignals(false);
    }
}

ResultForm::~ResultForm()
{
    delete ui;
}
