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
 
#ifndef COOKIESFORM_H
#define COOKIESFORM_H

#include <QWidget>
#include <QNetworkCookieJar>

#include "publiccookiejar.h"
#include "models/cookiesmodel.h"

namespace Ui {
class CookiesForm;
}

class CookiesForm : public QWidget
{
    Q_OBJECT

public:
    explicit CookiesForm(const PublicCookieJar *cookieJar, QWidget *parent = 0);
    ~CookiesForm();

protected slots:
    void updateCookies();

private:
    Ui::CookiesForm *ui;
    CookiesModel m_cookiesModel;
    QList<QNetworkCookie> m_cookies;
    const PublicCookieJar *m_cookieJar;
};

#endif // COOKIESFORM_H
