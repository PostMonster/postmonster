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
 
#include "editform.h"
#include "ui_editform.h"

#include "common.h"
#include "diagram/arrow.h"
#include "diagram/startitem.h"
#include "diagram/taskitem.h"

#include <QScrollBar>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QSettings>
#include <QPluginLoader>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QTimer>

using namespace PostMonster;

EditForm::EditForm(QWidget *parent) :
    QWidget(parent), ui(new Ui::EditForm),
    m_plugins(PluginRegistry::instance()),
    m_requestsModel(0), m_scene(0)
{
    ui->setupUi(this);

    ui->verticalSplitter->setStretchFactor(0, 5);
    ui->verticalSplitter->setStretchFactor(1, 3);

    QList<int> sizes = { int(width() * 0.65), int(width() * 0.35) };
    ui->debugSplitter->setSizes(sizes);

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(debugStep()));

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateRequestParams()));

    m_toolbar.setOrientation(Qt::Vertical);
    m_toolbar.setIconSize(QSize(32, 32));
    ui->horizontalLayout->addWidget(&m_toolbar);

    m_envModel.setHeaderData(0, Qt::Horizontal, "Name", Qt::DisplayRole);
    ui->environmentTree->setModel(&m_envModel);
    connect(ui->environmentTree, SIGNAL(expanded(QModelIndex)), this, SLOT(fitEnvColumns()));
    connect(ui->environmentTree, SIGNAL(collapsed(QModelIndex)), this, SLOT(fitEnvColumns()));
    connect(ui->environmentTree->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(environmentSelected(QModelIndex, QModelIndex)));

    ui->graphicsView->scale(Common::scale(), Common::scale());

    newProject();
}

void EditForm::openRequests()
{
    ui->tabWidget->setCurrentWidget(ui->requestsTab);
}

void EditForm::initScene()
{
    if (m_scene) {
        m_scene->destroyItems();
        delete m_scene;
    }

    m_scene = new DiagramScene(this);

    connect(m_scene, SIGNAL(itemInserted(DiagramItem *)), this, SLOT(resetMode()));
    connect(m_scene, SIGNAL(selectionChanged()), this, SLOT(itemSelected()));
    connect(m_scene, SIGNAL(requestDropped(int, QPointF)), this, SLOT(insertHttpItem(int, QPointF)));

    m_scene->setMode(DiagramScene::MoveItem);

    ui->graphicsView->setScene(m_scene);
}

void EditForm::resetEnvironment()
{
    for (int i = 1; i < ui->resultStackedWidget->count(); i++)
        ui->resultStackedWidget->removeWidget(ui->resultStackedWidget->widget(i));

    m_envModel.clear();
    m_engine.reset();
}

void EditForm::newProject()
{
    initScene();

    DiagramItem *item = new StartItem();
    m_scene->insertItem(item);

    // This is workaround for the QGraphicsScene behaviour when inserting the first item.
    // If we set center position immediately, scene rect will just translate its
    // coordinates to this position, and on the screen it will be at top left corner.
    // So we insert item at default position (0, 0) and on next event loop iteration
    // move it to the QGraphicsView center, so it will be displayed at viewport center.
    QTimer::singleShot(0, [this, item]() {
        QPointF pos = QPointF(ui->graphicsView->width() / 2.0 - item->boundingRect().width() / 2.0,
                              ui->graphicsView->height() / 2.0 - item->boundingRect().height() / 2.0);
        pos.setX(pos.x() / Common::scale());
        pos.setY(pos.y() / Common::scale());

        item->setPos(pos);
    });

    if (m_requestsModel)
        m_requestsModel->clear();

    resetEnvironment();
}

void EditForm::saveProject(const QString &fileName)
{
    QJsonObject result, scene;
    QJsonArray items, requests;

    scene.insert("top", m_scene->sceneRect().top());
    scene.insert("left", m_scene->sceneRect().left());
    scene.insert("width", m_scene->sceneRect().width());
    scene.insert("height", m_scene->sceneRect().height());
    result.insert("scene", scene);

    foreach (QGraphicsItem *item, m_scene->items()) {
        DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem *>(item);
        if (!diagramItem)
            continue;

        //const QList<Arrow *> *arrows = diagramItem->arrows();
        QJsonObject jsonItem;
        jsonItem.insert("uuid", diagramItem->uuid().toString());
        jsonItem.insert("top", item->pos().x());
        jsonItem.insert("left", item->pos().y());
        jsonItem.insert("zvalue", item->zValue());

        if (diagramItem->diagramType() == DiagramItem::TypeTask) {
            TaskItem *taskItem = static_cast<TaskItem *>(diagramItem);
            QJsonObject jsonTask;

            jsonTask.insert("tool", m_plugins.info(taskItem->task()->tool()).value("id"));
            jsonTask.insert("data", taskItem->task()->serialize());

            jsonItem.insert("type", "task");
            jsonItem.insert("task", jsonTask);
        } else if (diagramItem->diagramType() == DiagramItem::TypeStart)
            jsonItem.insert("type", "start");

        QJsonObject jsonArrows;
        for (QList<Arrow *>::const_iterator i = diagramItem->arrows()->constBegin(),
             end = diagramItem->arrows()->end(); i != end; ++i)
            jsonArrows.insert(QString::number((*i)->status()), (*i)->endItem()->uuid().toString());
        jsonItem.insert("out", jsonArrows);

        items << jsonItem;
    }
    result.insert("items", items);

    if (m_httpTool) {
        int requestCount = m_requestsModel->rowCount();
        for (int i = 0; i < requestCount; i++) {
            if (!m_requestsModel->used(i)) continue;

            QJsonObject request = Common::serializeRequest(m_requestsModel->request(i));
            QJsonObject response = Common::serializeResponse(m_requestsModel->response(i));

            QJsonObject requestItem;
            requestItem.insert("request", request);
            requestItem.insert("response", response);

            requests << requestItem;
        }
    }
    result.insert("requests", requests);

    QJsonDocument document;
    document.setObject(result);

    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(document.toJson());
    jsonFile.close();

    m_fileName = fileName;
}

void EditForm::loadProject(const QString &fileName)
{
    QJsonObject project;

    QFile jsonFile(fileName);
    jsonFile.open(QFile::ReadOnly);
    project = QJsonDocument().fromJson(jsonFile.readAll()).object();
    jsonFile.close();

    m_fileName = fileName;
    initScene();

    resetEnvironment();

    QJsonArray items = project.value("items").toArray();
    QHash< QUuid, QHash<PostMonster::TaskStatus, QUuid> > parsedArrows;
    QHash< QUuid, DiagramItem * > parsedItems;

    foreach (const QJsonValue &value, items) {
        QJsonObject jsonItem = value.toObject();

        QPointF pos(jsonItem.value("top").toDouble(),
                    jsonItem.value("left").toDouble());

        QString type = jsonItem.value("type").toString();
        qreal zValue = jsonItem.value("zvalue").toDouble();

        QUuid uuid = jsonItem.value("uuid").toVariant().toUuid();

        DiagramItem *item = 0;
        if (type == "start") {
            item = new StartItem;
        } else if (type == "task") {
            QJsonObject jsonTask = jsonItem.value("task").toObject();
            QJsonObject jsonData = jsonTask.value("data").toObject();

            QString name = jsonTask.value("tool").toString();
            PostMonster::ToolPluginInterface *tool = m_plugins.tool(name);
            if (tool) {
                PostMonster::TaskInterface *task = tool->deserializeTask(jsonData);
                if (!task)
                    continue;

                item = new TaskItem(task);
            }
        }

        if (!item)
            continue;

        QVariantMap jsonArrows = jsonItem.value("out").toObject().toVariantMap();
        QHash<PostMonster::TaskStatus, QUuid> arrow;
        for (QVariantMap::iterator i = jsonArrows.begin(),
             end = jsonArrows.end(); i != end; ++i) {
            QUuid destUuid = i.value().toUuid();

            if (item->diagramType() == DiagramItem::TypeTask) {
                PostMonster::TaskStatus status = static_cast<PostMonster::TaskStatus>(i.key().toInt());
                TaskItem *taskItem = static_cast<TaskItem *>(item);
                if (taskItem->tool()->statuses().contains(status))
                    arrow[status] = destUuid;
            } else
                arrow[PostMonster::Default] = destUuid;
        }

        if (zValue)
            item->setZValue(zValue);

        parsedArrows[uuid] = arrow;
        parsedItems[uuid] = item;

        item->setUuid(uuid);
        m_scene->insertItem(item);

        QTimer::singleShot(0, [item, pos]() { item->setPos(pos); });
    }

    for (QHash< QUuid, QHash<PostMonster::TaskStatus, QUuid> >::iterator i = parsedArrows.begin(),
         end = parsedArrows.end(); i != end; ++i) {
        DiagramItem *startItem = parsedItems[i.key()];

        for (QHash<PostMonster::TaskStatus, QUuid>::iterator j = i.value().begin(),
             end = i.value().end(); j != end; ++j) {
            if (!parsedItems.contains(j.value()))
                continue;

            PostMonster::TaskStatus status = j.key();
            DiagramItem *endItem = parsedItems[j.value()];

            m_scene->insertArrow(status, startItem, endItem);
        }
    }

    QJsonArray requests = project.value("requests").toArray();
    PostMonster::HttpToolPluginInterface *tool = m_httpTool;

    if (m_requestsModel) {
        m_requestsModel->clear();
        if (tool && !requests.isEmpty()) {
            foreach (const QJsonValue &value, requests) {
                QJsonObject jsonRequest = value.toObject().value("request").toObject();
                QJsonObject jsonResponse = value.toObject().value("response").toObject();

                HttpRequest *request = new HttpRequest(Common::deserializeRequest(jsonRequest));
                HttpResponse *response = new HttpResponse(Common::deserializeResponse(jsonResponse));

                m_requestsModel->add(request, response, true);
            }
        }
    }
}

void EditForm::actionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    PostMonster::ToolPluginInterface *tool = qobject_cast<PostMonster::ToolPluginInterface *>(action->parent());

    if (!tool || !m_scene) return;

    resetMode(action);
    if (action->isChecked())
        m_scene->setMode(DiagramScene::InsertItem, tool);
}

void EditForm::setRequestsModel(PostMonster::HttpToolPluginInterface *httpTool, RequestsModel *model)
{
    m_httpTool = httpTool;
    m_requestsModel = model;

    ui->requestsForm->setRequestsModel(model);
    ui->requestsForm->setHttpTool(httpTool);

    connect(ui->environmentRadio, SIGNAL(toggled(bool)), this, SLOT(toggleDebugStackedWidget()));
    connect(ui->cookiesRadio, SIGNAL(toggled(bool)), this, SLOT(toggleDebugStackedWidget()));
}

void EditForm::addToolToToolbar(QObject *plugin)
{
    PostMonster::ToolPluginInterface *tool = qobject_cast<PostMonster::ToolPluginInterface *>(plugin);
    if (!tool) return;

    QString name = m_plugins.info(tool).value("name").toString();
    QAction *action = new QAction(tool->icon(), name, plugin);

    action->setCheckable(true);
    connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));

    m_toolbar.addAction(action);
}

void EditForm::itemSelected()
{
    ui->stackedWidget->setCurrentIndex(0);
    for (int i = 1; i < ui->stackedWidget->count(); i++)
        ui->stackedWidget->removeWidget(ui->stackedWidget->widget(i));

    if (!m_scene->selectedItems().count())
        return;

    DiagramItem *item = qgraphicsitem_cast<DiagramItem *> (m_scene->selectedItems().at(0));
    if (!item) return;

    if (item->diagramType() == DiagramItem::TypeTask) {
        TaskItem *taskItem = static_cast<TaskItem *> (item);
        QWidget *widget = taskItem->task()->widget();

        ui->stackedWidget->addWidget(widget);
        ui->stackedWidget->setCurrentWidget(widget);
        ui->stackedWidget->currentWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    }
}

void EditForm::updateRequestParams()
{
    if (ui->tabWidget->currentWidget() == ui->requestsTab)
        ui->requestsForm->updateResponse();
    else if (ui->tabWidget->currentWidget() == ui->debugTab)
        environmentSelected(ui->environmentTree->selectionModel()->currentIndex(),
                        QModelIndex());
}

void EditForm::resetMode(QAction *except)
{
    m_scene->setMode(DiagramScene::MoveItem);

    foreach (QAction *action, m_toolbar.actions()) {
        if (action != except)
            action->setChecked(false);
    }
}

void EditForm::insertHttpItem(int requestRow, QPointF scenePos)
{
    if (requestRow >= m_requestsModel->rowCount()) return;

    HttpRequest *request = m_requestsModel->request(requestRow);
    HttpToolPluginInterface *tool = m_httpTool;

    if (tool) {
        TaskInterface *task = tool->createTask(*request);
        DiagramItem *item = new TaskItem(task);

        item->setPos(scenePos);
        item->setSelected(true);

        m_scene->clearSelection();
        m_scene->insertItem(item);
    }
}

void EditForm::fitEnvColumns()
{
    ui->environmentTree->resizeColumnToContents(0);
}

void EditForm::environmentSelected(const QModelIndex &newIndex, const QModelIndex &oldIndex)
{
    Q_UNUSED(oldIndex)

    QModelIndex toolIndex = m_envModel.index(newIndex.row(), EnvironmentModel::Name, newIndex.parent());
    QModelIndex taskIndex;
    while (m_envModel.parent(toolIndex).isValid()) {
        taskIndex = toolIndex;
        toolIndex = m_envModel.parent(taskIndex);
    };

    QString toolName = toolIndex.data().toString();
    QString taskName = taskIndex.data().toString();

    if (!m_plugins.tool(toolName)) return;

    PostMonster::TaskInterface *task = m_engine.task(toolName, taskName);
    QWidget *widget = task ? task->resultWidget() : 0;

    if (widget/* && ui->resultStackedWidget->property("uuid").toUuid() != task->uuid()*/) {
        for (int i = 1; i < ui->resultStackedWidget->count(); i++)
            ui->resultStackedWidget->removeWidget(ui->resultStackedWidget->widget(i));

        //ui->resultStackedWidget->setProperty("uuid", task->uuid());
        ui->resultStackedWidget->addWidget(widget);
        ui->resultStackedWidget->setCurrentWidget(widget);
        ui->resultStackedWidget->currentWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    }
}

void EditForm::debugStep()
{
    if (!m_scene->selectedItems().count())
        return;

    DiagramItem *item = qgraphicsitem_cast<DiagramItem *> (m_scene->selectedItems().at(0));
    if (!item) return;

    m_engine.setActiveItem(item);
    m_engine.step();

    m_scene->clearSelection();
    if (m_engine.activeItem())
        m_engine.activeItem()->setSelected(true);

    m_envModel.loadJson(*m_engine.environment());

    if (item->diagramType() == DiagramItem::TypeTask) {
        TaskItem *taskItem = static_cast<TaskItem *>(item);
        QString toolName = m_plugins.info(taskItem->task()->tool()).value("id").toString();

        int rowCount = m_envModel.rowCount();
        for (int i = 0; i < rowCount; ++i) {
            QModelIndex toolIndex = m_envModel.index(i, 0);

            if (m_envModel.data(toolIndex).toString() == toolName) {
                rowCount = m_envModel.rowCount(toolIndex);
                for (i = 0; i < rowCount; ++i) {
                    QModelIndex taskIndex = m_envModel.index(i, 0, toolIndex);
                    if (m_envModel.data(taskIndex).toString() == taskItem->task()->name()) {
                        ui->environmentTree->setCurrentIndex(taskIndex);
                        ui->environmentTree->expand(taskIndex);
                        break;
                    }
                }
                break;
            }
        }
    }
}

void EditForm::toggleDebugStackedWidget()
{
    ui->debugStackedWidget->setCurrentWidget(ui->environmentPage);
}

EditForm::~EditForm()
{
    //TODO UNLOAD PLUGINS?
    m_scene->destroyItems();
    delete m_scene;
    delete ui;
}
