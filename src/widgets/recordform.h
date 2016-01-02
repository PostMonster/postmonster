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
 
#ifndef RECORDFORM_H
#define RECORDFORM_H

#include <QWidget>

#include "../networksniffer.h"
#include "models/requestsmodel.h"

#include "toolplugin.h"

namespace Ui {
class RecordForm;
}

class RecordForm : public QWidget
{
    Q_OBJECT

public:
    explicit RecordForm(QWidget *parent = 0);
    void setRequestsModel(PostMonster::HttpToolPluginInterface *httpTool, RequestsModel *model);
    ~RecordForm();

public slots:
    void updateRequestParams();

signals:
    void recordFinished();

private:
    Ui::RecordForm *ui;
    NetworkSniffer m_networkSniffer;
    QStringList userInput;
    bool m_recording;

private slots:
    void browserLoad();
    void requestFinished(QNetworkReply *reply,
                         const QByteArray &requestData, const QByteArray &replyData);
    void startRecord();
    void stopRecord();
    void blinkRecordButton();

protected:
    void showEvent(QShowEvent *event);
};

#endif // RECORDFORM_H
