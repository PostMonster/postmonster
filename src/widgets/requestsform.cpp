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
 
#include "requestsform.h"
#include "ui_requestsform.h"

#include "checkboxheader.h"
#include "checkboxdelegate.h"

using namespace PostMonster;

RequestsForm::RequestsForm(QWidget *parent) :
    QWidget(parent), m_httpTool(0), m_useAllRequests(false),
    ui(new Ui::RequestsForm)
{
    ui->setupUi(this);

    QList<int> sizes = { int(width() * 0.65), int(width() * 0.35) };
    ui->splitter->setSizes(sizes);

    CheckBoxHeader *header = new CheckBoxHeader(RequestsModel::Used,
                                                Qt::Horizontal,
                                                ui->requestsTable);
    ui->requestsTable->setHorizontalHeader(header);
    ui->requestsTable->setDragEnabled(false);
    ui->requestsTable->installEventFilter(this);

    connect(header, SIGNAL(checkBoxClicked(bool)), this, SLOT(toggleAllRequests(bool)));

    connect(ui->typesCBox,    SIGNAL(currentTextChanged(QString)), this, SLOT(applyFilters()));
    connect(ui->statusesCBox, SIGNAL(currentTextChanged(QString)), this, SLOT(applyFilters()));
    connect(ui->urlEdit,      SIGNAL(textChanged(QString)),        this, SLOT(applyFilters()));
}

void RequestsForm::applyFilters()
{
    //ui->typesCBox->lineEdit()->setCursorPosition(0);

    m_proxyModel.setFilter(RequestsModel::URL,  "*" + ui->urlEdit->text() + "*");
    m_proxyModel.setFilter(RequestsModel::Type, "*" + ui->typesCBox->currentText() + "*");
    m_proxyModel.setFilter(RequestsModel::Code, ui->statusesCBox->currentText());

    m_proxyModel.invalidate();
}

bool RequestsForm::isRowUsed(int row)
{
    Q_UNUSED(row);
    return true;
}

void RequestsForm::updateFilters()
{
    int currentType = ui->typesCBox->currentIndex();
    for (int i = ui->typesCBox->count() - 1; i > 0; --i) {
        if (currentType != i)
            ui->typesCBox->removeItem(i);
    }

    int currentStatus = ui->statusesCBox->currentIndex();
    for (int i = ui->statusesCBox->count() - 1; i > 0; --i) {
        if (currentStatus != i)
            ui->statusesCBox->removeItem(i);
    }

    RequestsModel *model = static_cast<RequestsModel *>(m_proxyModel.sourceModel());
    int rowCount = model->rowCount();

    int k = 1, m = 1;
    bool statusFound = false, typeFound = false;
    for (int i = 0; i < rowCount; ++i) {
        if (!isRowUsed(i)) continue;

        int status   = model->data(model->index(i, RequestsModel::Code)).toInt();
        QString type = model->data(model->index(i, RequestsModel::Type)).toString();

        bool found = false;
        int count = ui->typesCBox->count();
        for (int j = 0; j < count && !found; ++j)
            if (ui->typesCBox->itemText(j) == type)
                found = true;

        if (!found) {
            if (k == currentType) k++;
            ui->typesCBox->insertItem(k++, type);
        }

        if (!type.isEmpty() && type == ui->typesCBox->currentText())
            typeFound = true;

        found = false;
        count = ui->statusesCBox->count();
        for (int j = 0; j < count && !found; ++j)
            if (ui->statusesCBox->itemData(j).toInt() == status)
                found = true;

        if (!found) {
            QPixmap icon(16, 16);
            icon.fill(Qt::transparent);

            QPainter painter(&icon);
            QColor color = model->data(model->index(i, RequestsModel::Code),
                                       Qt::BackgroundColorRole).value<QColor>();
            painter.setPen(Qt::transparent);
            painter.setBrush(color);
            painter.drawEllipse(0, 0, 16, 16);

            if (m == currentStatus) m++;
            ui->statusesCBox->insertItem(m++, QIcon(icon), QString::number(status), status);
        }
        if (status == ui->statusesCBox->currentData().toInt())
            statusFound = true;
    }

    if (!typeFound) {
        ui->typesCBox->setCurrentIndex(0);
        if (currentType > 0)
            ui->typesCBox->removeItem(currentType);
    }
    if (!statusFound) {
        ui->statusesCBox->setCurrentIndex(0);
        if (currentStatus > 0)
            ui->statusesCBox->removeItem(currentStatus);
    }
}

void RequestsForm::setRequestsModel(RequestsModel *model)
{
    m_proxyModel.setSourceModel(model);

    ui->requestsTable->setModel(&m_proxyModel);
    connect(ui->requestsTable->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this,
            SLOT(updateResponse()), Qt::UniqueConnection);

    connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SLOT(updateFilters()), Qt::UniqueConnection);
    connect(model, SIGNAL(rowsRemoved(QModelIndex, int, int)),
            this, SLOT(updateFilters()), Qt::UniqueConnection);
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(updateFilters()), Qt::UniqueConnection);

    ui->requestsTable->resizeColumnsToContents();
    ui->requestsTable->setColumnWidth(RequestsModel::Type, 110);
    ui->requestsTable->horizontalHeader()->setSectionResizeMode(RequestsModel::URL,
                                                                QHeaderView::Stretch);

    CheckBoxDelegate *delegate = new CheckBoxDelegate(ui->requestsTable);
    ui->requestsTable->setItemDelegateForColumn(RequestsModel::Used, delegate);
}

QTableView *RequestsForm::requestsTable()
{
    return ui->requestsTable;
}

void RequestsForm::add(HttpRequest *request, HttpResponse *response)
{
    RequestsModel *model = qobject_cast<RequestsModel *> (m_proxyModel.sourceModel());
    if (!model) return;

    model->add(request, response, m_useAllRequests);
    if (model->rowCount() > 0) {
        //ui->requestsTable->resizeColumnToContents(RequestsModel::Type);
        ui->requestsTable->selectRow(model->rowCount() - 1);
    }
}

void RequestsForm::setHttpTool(PostMonster::HttpToolPluginInterface *httpTool)
{
    m_httpTool = httpTool;

    ui->requestsTable->setDragPixmap(httpTool->icon());
}

void RequestsForm::updateResponse()
{
    ui->stackedWidget->setCurrentIndex(0);
    for (int i = 1; i < ui->stackedWidget->count(); i++)
        ui->stackedWidget->removeWidget(ui->stackedWidget->widget(i));

    if (!m_httpTool || !ui->requestsTable->selectionModel()->hasSelection())
        return;

    QModelIndex index = m_proxyModel.mapToSource(m_proxyModel.index(
        ui->requestsTable->selectionModel()->selectedRows().at(0).row(), 0
    ));

    RequestsModel *model = static_cast<RequestsModel *>(m_proxyModel.sourceModel());
    HttpRequest *request = model->request(index.row());
    HttpResponse *response = model->response(index.row());

    QWidget *widget = m_httpTool->resultWidget(request, response);
    ui->stackedWidget->addWidget(widget);
    ui->stackedWidget->setCurrentWidget(widget);
    ui->stackedWidget->currentWidget()->layout()->setContentsMargins(0, 0, 0, 0);
}

void RequestsForm::toggleAllRequests(bool used)
{
    m_useAllRequests = used;

    int count = m_proxyModel.rowCount();
    for (int i = 0; i < count; ++i)
        m_proxyModel.setData(m_proxyModel.index(i, RequestsModel::Used), used);
}

void RequestsForm::removeRequests(const QModelIndexList &indexes)
{
    QModelIndexList::const_iterator i = indexes.constEnd();
    while (i != indexes.constBegin()) {
        --i;
        m_proxyModel.removeRow((*i).row());
    }
}

bool RequestsForm::eventFilter(QObject *obj, QEvent *event)
{
    QTableView *table = qobject_cast<QTableView *>(obj);

    if (table && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            QModelIndexList indexes = table->selectionModel()->selectedRows();
            removeRequests(indexes);
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}

RequestsForm::~RequestsForm()
{
    delete ui;
}
