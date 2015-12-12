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
 
#ifndef REQUESTSFORM_H
#define REQUESTSFORM_H

#include <QWidget>
#include <QTableView>

#include "models/requestsmodel.h"
#include "models/requestsproxymodel.h"

#include "toolplugin.h"

namespace Ui {
class RequestsForm;
}

class RequestsForm : public QWidget
{
    Q_OBJECT

public:
    explicit RequestsForm(QWidget *parent = 0);
    void add(PostMonster::HttpRequest *request, PostMonster::HttpResponse *response);
    virtual void setRequestsModel(RequestsModel *model);
    void setHttpTool(PostMonster::HttpToolPluginInterface *httpTool);
    virtual ~RequestsForm();

public slots:
    void updateResponse();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    virtual bool isRowUsed(int row);
    virtual void removeRequests(const QModelIndexList &indexes);

    QTableView *requestsTable();
    RequestsProxyModel m_proxyModel;

protected slots:
    void toggleAllRequests(bool used);
    void applyFilters();
    void updateFilters();

private:
    PostMonster::HttpToolPluginInterface *m_httpTool;
    Ui::RequestsForm *ui;
};

#endif // REQUESTSFORM_H
