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
 
#ifndef REQUESTFORM_H
#define REQUESTFORM_H

#include <QWidget>

#include "httptoolplugin.h"
#include "models/headersmodel.h"
#include "models/cookiesmodel.h"

namespace Ui {
class RequestForm;
}

class HttpTask;

class RequestForm : public QWidget
{
    Q_OBJECT

public:
    explicit RequestForm(QWidget *parent = 0);
    ~RequestForm();

    void updateData(HttpTask *task, const QStringList &encodings = QStringList());

protected slots:
    void updateTask();
    void addCookie();
    void addHeader();
    void removeCookies();
    void removeHeaders();

private:
    Ui::RequestForm *ui;
    HeadersModel m_headersModel;
    CookiesModel m_cookiesModel;
    HttpTask *m_task;
};

#endif // REQUESTFORM_H
