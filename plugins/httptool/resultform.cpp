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

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(renderData()));

    connect(ui->encodingCBox, SIGNAL(currentTextChanged(QString)),
            this, SLOT(encodingChanged(QString)));
}

void ResultForm::encodingChanged(const QString &encoding)
{
    QTextCodec *codec = QTextCodec::codecForName(encoding.isEmpty() ? "Latin1" : encoding.toLatin1());
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());

    if (codec) {
        if (comboBox)
            comboBox->setStyleSheet("");

        if (ui->requestRadio->isChecked())
            ui->bodyText->setPlainText(codec->toUnicode(m_request->body));
        else
            ui->bodyText->setPlainText(codec->toUnicode(m_response->body));
    } else if (comboBox)
       comboBox->setStyleSheet("QComboBox:editable:!on { background: #FF7777; color: white }");
}

void ResultForm::updateData(const HttpRequest *request, const HttpResponse *response)
{
    m_request = request;
    m_response = response;

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
        QString html = "<table align='center'>";
        foreach (QNetworkReply::RawHeaderPair header, *headers) {
            html += QString("<tr><td align='right' nowrap><u>%1</u>:</td><td>%2</td></tr>").
                    arg(QString(header.first)).arg(QString(header.second));
        }
        html += "</table>";
        ui->headersText->setHtml(html);
    }

    if (cookies->isEmpty()) {
        ui->tabWidget->setTabEnabled(1, false);
        ui->cookiesText->clear();
    } else
        ui->tabWidget->setTabEnabled(1, true);

    if (ui->tabWidget->currentWidget() == ui->cookiesTab) {
        QString html = "<table align='center'>";
        foreach (QNetworkCookie cookie, *cookies) {
            QString name(cookie.name());
            QString value(cookie.toRawForm().mid(name.length() + 1));
            html += QString("<tr><td align='right' nowrap><u>%1</u>:</td><td>%2</td></tr>").
                    arg(name).arg(value);
        }
        html += "</table>";
        ui->cookiesText->setHtml(html);
    }

    if (body->isEmpty()) {
        ui->tabWidget->setTabEnabled(2, false);
    } else
        ui->tabWidget->setTabEnabled(2, true);

    if (ui->tabWidget->currentWidget() == ui->bodyTab) {
        QString encoding;
        foreach (QNetworkReply::RawHeaderPair header, *headers) {
            const QString &headerName = QString(header.first).toLower();
            const QString &headerValue = QString(header.second).toLower();

            if (headerName == "content-type" &&
                    (headerValue.startsWith("application/x-www-form-urlencoded;") ||
                     headerValue.startsWith("multipart/form-data;") ||
                     headerValue.startsWith("text/"))) {
                QRegExp charsetRx("charset\\=([^;]+)", Qt::CaseInsensitive);
                if (charsetRx.indexIn(headerValue) != -1) {
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

        ui->encodingCBox->blockSignals(true);
        ui->encodingCBox->clear();
        ui->encodingCBox->insertItems(0, encodings);
        ui->encodingCBox->blockSignals(false);

        ui->encodingCBox->setCurrentText(encoding);
    }

    ui->tabWidget->blockSignals(false);
}

ResultForm::~ResultForm()
{
    delete ui;
}
