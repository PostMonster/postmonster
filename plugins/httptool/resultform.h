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
 
#ifndef RESULTFORM_H
#define RESULTFORM_H

#include <QWidget>

#include "httptoolplugin.h"

namespace Ui {
class ResultForm;
}

class ResultForm : public QWidget
{
    Q_OBJECT

public:
    explicit ResultForm(QWidget *parent = 0);
    ~ResultForm();

    void updateData(const HttpRequest *request, const HttpResponse *response);

private slots:
    void requestEncodingChanged(const QString &encoding);
    void responseEncodingChanged(const QString &encoding);
    void renderData();

private:
    Ui::ResultForm *ui;
    const HttpRequest *m_request;
    const HttpResponse *m_response;
    static const QStringList encodings;
};

#endif // RESULTFORM_H
