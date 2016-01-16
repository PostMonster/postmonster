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

#include "recordform.h"
#include "ui_recordform.h"

#include <QWebFrame>
#include <QWebElementCollection>
#include <QTimer>
#include <QTextCodec>

#include "common.h"

using namespace PostMonster;

RecordForm::RecordForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecordForm),
    m_recording(false)
{
    ui->setupUi(this);

    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 4);

    connect(ui->urlEdit, SIGNAL(editingFinished()), this, SLOT(browserLoad()));
    connect(ui->backButton, SIGNAL(clicked()), ui->webView, SLOT(back()));
    connect(ui->forwardButton, SIGNAL(clicked()), ui->webView, SLOT(forward()));
    connect(ui->reloadButton, SIGNAL(clicked()), ui->webView, SLOT(reload()));

    ui->webView->settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, false);
    ui->webView->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
    ui->webView->settings()->setAttribute(QWebSettings::JavascriptCanCloseWindows, true);

    // Disable caching
    ui->webView->settings()->setMaximumPagesInCache(0);
    ui->webView->settings()->setObjectCacheCapacities(0, 0, 0);

    ui->webView->page()->setNetworkAccessManager(&m_networkSniffer);
    connect(&m_networkSniffer, SIGNAL(replyReceived(QNetworkReply *, QByteArray, QByteArray)),
            this, SLOT(requestFinished(QNetworkReply *, QByteArray, QByteArray)));

    ui->urlEdit->setWebView(ui->webView);

    stopRecord();
}

void RecordForm::setRequestsModel(PostMonster::HttpToolPluginInterface *httpTool, RequestsModel *model)
{
    ui->requestsForm->setRequestsModel(model);
    ui->requestsForm->setHttpTool(httpTool);
}

void RecordForm::updateRequestParams()
{
    ui->requestsForm->updateResponse();
}

void RecordForm::showEvent(QShowEvent *event)
{
    if (ui->urlEdit->text().isEmpty())
        ui->urlEdit->setFocus();
    QWidget::showEvent(event);
}

void RecordForm::startRecord()
{
    disconnect(ui->recordButton, SIGNAL(clicked()), this, SLOT(startRecord()));
    connect(ui->recordButton, SIGNAL(clicked()), this, SLOT(stopRecord()));

    m_recording = true;
    blinkRecordButton();

    ui->webView->setFocus();
}

void RecordForm::stopRecord()
{
    m_recording = false;

    disconnect(ui->recordButton, SIGNAL(clicked()), this, SLOT(stopRecord()));
    connect(ui->recordButton, SIGNAL(clicked()), this, SLOT(startRecord()));

    emit recordFinished();
}

void RecordForm::blinkRecordButton()
{
    if (!m_recording) {
        ui->recordButton->setProperty("blink", false);
        ui->recordButton->setIcon(QIcon(":/icons/record"));
        return;
    }

    QVariant blink = ui->recordButton->property("blink");
    if (!blink.isNull() && blink.toBool()) {
        ui->recordButton->setProperty("blink", false);
        ui->recordButton->setIcon(QIcon(":/icons/record"));
        QTimer::singleShot(1000, this, SLOT(blinkRecordButton()));
    } else {
        ui->recordButton->setProperty("blink", true);
        ui->recordButton->setIcon(QIcon(":/icons/record-dim"));
        QTimer::singleShot(500, this, SLOT(blinkRecordButton()));
    }
}

void RecordForm::requestFinished(QNetworkReply *reply, const QByteArray &requestData, const QByteArray &replyData)
{
    if (!m_recording || reply->url().host().isEmpty())
        return;

    HttpRequest *request = new HttpRequest();
    request->url = reply->url().url();
    request->body = requestData;

    switch (reply->operation()) {
    case QNetworkAccessManager::HeadOperation:
        request->method = "HEAD";
        break;
    case QNetworkAccessManager::PutOperation:
        request->method = "PUT";
        break;
    case QNetworkAccessManager::PostOperation:
        request->method = "POST";
        break;
    case QNetworkAccessManager::DeleteOperation:
        request->method = "DELETE";
        break;
    default:
        request->method = "GET";
        break;
    }

    HttpResponse *response = new HttpResponse();
    response->status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    response->body = replyData;

    foreach (const QNetworkReply::RawHeaderPair &pair, reply->rawHeaderPairs()) {
        const QString &headerName = QLatin1String(pair.first);
        if (!headerName.compare("cookie", Qt::CaseInsensitive))
            continue;

        response->headers << pair;
    }

    response->mimeType = reply->header(QNetworkRequest::ContentTypeHeader).toByteArray();
    const int semicolonPos = response->mimeType.indexOf(';');
    if (semicolonPos != -1)
        response->mimeType.truncate(semicolonPos);

    response->cookies = reply->header(QNetworkRequest::SetCookieHeader).
            value< QList<QNetworkCookie> >();

    foreach (const QByteArray &name, reply->request().rawHeaderList()) {
        QPair<QByteArray, QByteArray> pair;
        pair.first = name;
        pair.second = reply->request().rawHeader(name);

        const QString &headerName = QString(pair.first).toLower();
        if (headerName == "cookie") {
            continue;
        } else if (headerName == "content-type") {
            QRegExp charsetRx("charset\\=([^;]+)", Qt::CaseInsensitive);
            if (charsetRx.indexIn(pair.second) != -1) {
                request->encoding = charsetRx.cap(1).toLatin1();
            }
        }

        request->headers << pair;
    }

    ui->requestsForm->add(request, response);

    reply->deleteLater();
}

void RecordForm::browserLoad()
{
    ui->webView->setFocus();
    ui->webView->load(QUrl::fromUserInput(ui->urlEdit->text()));
}

RecordForm::~RecordForm()
{
    delete ui;
}
