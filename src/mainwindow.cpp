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
 
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QToolButton>
#include <QFileDialog>
#include <QSettings>

#include "common.h"
#include "pluginsdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_plugins(PluginRegistry::instance())
{
    ui->setupUi(this);

    connect(ui->actionEdit, SIGNAL(triggered(bool)), this, SLOT(openEdit()));
    connect(ui->actionRecord, SIGNAL(triggered(bool)), this, SLOT(openRecord()));
    connect(ui->actionPlugins, SIGNAL(triggered(bool)), this, SLOT(openPlugins()));

    connect(ui->recordPage, SIGNAL(recordFinished()), this, SLOT(editRequests()));

    QToolButton *openButton = static_cast<QToolButton *>(ui->toolBar->widgetForAction(ui->actionOpen));
    openButton->setPopupMode(QToolButton::MenuButtonPopup);
    QAction *openSeparator = new QAction(this);
    openSeparator->setSeparator(true);
    openButton->addAction(openSeparator);

    QToolButton *saveButton = static_cast<QToolButton *>(ui->toolBar->widgetForAction(ui->actionSave));
    saveButton->setPopupMode(QToolButton::MenuButtonPopup);
    saveButton->addAction(ui->actionSaveAs);

    connect(ui->actionNew,    SIGNAL(triggered(bool)), ui->editPage, SLOT(newProject()));
    connect(ui->actionOpen,   SIGNAL(triggered(bool)), this,         SLOT(loadProject()));
    connect(ui->actionSave,   SIGNAL(triggered(bool)), this,         SLOT(saveProject()));
    connect(ui->actionSaveAs, SIGNAL(triggered(bool)), this,         SLOT(saveProjectAs()));

    selectAction(ui->actionEdit);

    updateOpenMenu();

    connect(&m_plugins, SIGNAL(toolPluginLoaded(QObject *)),
            ui->editPage, SLOT(addToolToToolbar(QObject *)));

    PostMonster::APIFunctions api;
    api.evalScript          = WorkEngine::evalScript;
    api.serializeRequest    = Common::serializeRequest;
    api.deserializeRequest  = Common::deserializeRequest;
    api.serializeResponse   = Common::serializeResponse;
    api.deserializeResponse = Common::deserializeResponse;
    api.defaultEncodings    = Common::encodings;
    api.dpiScaleFactor         = Common::dpiScaleFactor;
    m_plugins.loadPlugins(api);

    if (m_plugins.tool("http")) {
        PostMonster::HttpToolPluginInterface *httpTool =
                static_cast<PostMonster::HttpToolPluginInterface *>(m_plugins.tool("http"));

        ui->editPage->setRequestsModel(httpTool, &m_requestsModel);
        ui->recordPage->setRequestsModel(httpTool, &m_requestsModel);
    }

    connect(ui->editPage, &EditForm::debugStatusChanged, ui->actionNew, &QAction::setDisabled);
    connect(ui->editPage, &EditForm::debugStatusChanged, ui->actionOpen, &QAction::setDisabled);
    connect(ui->editPage, &EditForm::debugStatusChanged, ui->actionRecord, &QAction::setDisabled);

    foreach (QToolBar *toolBar, findChildren<QToolBar *>()) {
        toolBar->setIconSize(toolBar->iconSize() * Common::dpiScaleFactor());
    }
}

void MainWindow::updateOpenMenu()
{
    QSettings settings;
    QStringList recentFiles = settings.value("recentFiles").toStringList();

    QToolButton *openButton =
            static_cast<QToolButton *>(ui->toolBar->widgetForAction(ui->actionOpen));

    foreach (QAction *action, openButton->actions()) {
        if (!action->property("recentFile").isNull())
            openButton->removeAction(action);
    }

    foreach (const QString &filePath, recentFiles) {
        QFileInfo fileInfo(filePath);

        QAction *action = new QAction(fileInfo.fileName(), openButton);
        action->setProperty("recentFile", filePath);
        connect(action, SIGNAL(triggered()), this, SLOT(loadRecentProject()));

        openButton->addAction(action);
    }
}

void MainWindow::loadProject()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select project file"), QDir::currentPath(),
        tr("PostMonster files (*.pms)"));

    if (fileName.isEmpty()) return;

    QSettings settings;
    QStringList recentFiles = settings.value("recentFiles").toStringList();
    if (recentFiles.count() > 5)
        recentFiles.removeLast();
    recentFiles.removeAll(fileName);
    recentFiles.insert(0, fileName);
    settings.setValue("recentFiles", recentFiles);

    ui->editPage->loadProject(fileName);

    updateOpenMenu();
}

void MainWindow::loadRecentProject()
{
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if (!action) return;

    QString fileName = action->property("recentFile").toString();
    if (!fileName.isEmpty())
        ui->editPage->loadProject(fileName);
}

void MainWindow::saveProject()
{
    const QString &fileName = ui->editPage->projectFile();

    if (fileName.isEmpty())
        saveProjectAs();
    else
        ui->editPage->saveProject(fileName);
}

void MainWindow::saveProjectAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Select project file"), QDir::currentPath(),
        tr("PostMonster files (*.pms)"));

    if (!fileName.isEmpty())
        ui->editPage->saveProject(fileName);
}

void MainWindow::openPlugins()
{
    PluginsDialog pluginsDlg;
    pluginsDlg.exec();
}

void MainWindow::selectAction(QAction *except)
{
    foreach (QAction *action, ui->toolBar->actions()) {
        if (action != except)
            action->setChecked(false);
    }

    except->setChecked(true);
}

void MainWindow::openEdit()
{
    selectAction(ui->actionEdit);
    ui->editPage->updateRequestParams();
    ui->stackedWidget->setCurrentWidget(ui->editPage);
}

void MainWindow::openRecord()
{
    selectAction(ui->actionRecord);

    ui->recordPage->updateRequestParams();
    ui->stackedWidget->setCurrentWidget(ui->recordPage);
}

void MainWindow::editRequests()
{
    ui->editPage->openRequests();
}

MainWindow::~MainWindow()
{
    delete ui;
}
