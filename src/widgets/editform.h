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
 
#ifndef EDITFORM_H
#define EDITFORM_H

#include <QWidget>

#include "workengine.h"
#include "models/requestsmodel.h"
#include "models/environmentmodel.h"
#include "diagram/diagramscene.h"
#include "postmonster.h"
#include "pluginregistry.h"

#include <QToolBar>
#include <QXmlStreamWriter>

namespace Ui {
class EditForm;
}

class EditForm : public QWidget
{
    Q_OBJECT

public:
    explicit EditForm(QWidget *parent = 0);
    ~EditForm();

    void setRequestsModel(PostMonster::HttpToolPluginInterface *httpTool, RequestsModel *model);
    const QString &projectFile() { return m_fileName; }

public slots:
    void addToolToToolbar(QObject *plugin);
    void newProject();
    void saveProject(const QString &fileName);
    void loadProject(const QString &fileName);
    void openRequests();
    void updateRequestParams();

protected slots:
    void selectionChanged();
    void currentItemChanged();
    void insertHttpItem(int requestRow, QPointF scenePos);
    void handleInsertedItem(DiagramItem *item);

    void envItemSelected(const QModelIndex &newIndex, const QModelIndex &oldIndex);
    void fitEnvColumns();
    void toggleDebugStackedWidget();
    void resetMode(QAction *except = 0);
    void actionTriggered();

    void resetWorker();
    void workerReady(DiagramItem *item);

    void debugRun();
    void debugStep();
    void debugPause();
    void debugStop();

protected:
    void initScene();
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    Ui::EditForm *ui;
    PluginRegistry &m_plugins;
    RequestsModel *m_requestsModel;
    EnvironmentModel m_envModel;
    DiagramScene *m_scene;
    QToolBar m_toolbar;
    QString m_fileName;
    WorkEngine m_engine;
    PostMonster::HttpToolPluginInterface *m_httpTool;
    QThread m_workerThread;
    bool m_debugRunning;
};

#endif // EDITFORM_H
