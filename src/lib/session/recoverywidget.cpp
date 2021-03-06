/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012 Franz Fellner <alpine.art.de@googlemail.com>
*                         David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
#include "recoverywidget.h"
#include "ui_recoverywidget.h"
#include "restoremanager.h"
#include "mainapplication.h"
#include "webview.h"
#include "qupzilla.h"

RecoveryWidget::RecoveryWidget(WebView* view, QupZilla* mainClass)
    : QWidget()
    , ui(new Ui::RecoveryWidget)
    , m_restoreManager(mApp->restoreManager())
    , m_view(view)
    , p_QupZilla(mainClass)
{
    ui->setupUi(this);

    setCursor(Qt::ArrowCursor);

    const RestoreData &data = m_restoreManager->restoreData();

    for (int i = 0; i < data.size(); ++i) {
        const RestoreManager::WindowData &wd = data.at(i);

        QTreeWidgetItem* root = new QTreeWidgetItem(ui->treeWidget);
        root->setText(0, tr("Window %1").arg((i + 1)));
        root->setCheckState(0, Qt::Checked);

        for (int tab = 0; tab < wd.tabsState.size(); ++tab) {
            const WebTab::SavedTab &st = wd.tabsState.at(tab);

            QTreeWidgetItem* child = new QTreeWidgetItem(root);
            child->setCheckState(0, Qt::Checked);
            child->setIcon(0, st.icon);
            child->setText(0, st.title);
        }
    }

    ui->treeWidget->expandAll();

    connect(ui->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), SLOT(onItemChanged(QTreeWidgetItem*, int)));
    connect(ui->restoreSession, SIGNAL(clicked()), this, SLOT(restoreSession()));
    connect(ui->newSession, SIGNAL(clicked()), this, SLOT(newSession()));
}

void RecoveryWidget::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (column != 0 || item->childCount() == 0) {
        return;
    }

    bool disabled = item->checkState(0) == Qt::Unchecked;

    for (int i = 0; i < item->childCount(); ++i) {
        item->child(i)->setDisabled(disabled);
    }
}

void RecoveryWidget::restoreSession()
{
    RestoreData data = m_restoreManager->restoreData();

    for (int win = ui->treeWidget->topLevelItemCount() - 1; win >= 0; --win) {
        QTreeWidgetItem* root = ui->treeWidget->topLevelItem(win);
        if (root->checkState(0) == Qt::Unchecked) {
            data.removeAt(win);
            continue;
        }

        RestoreManager::WindowData &wd = data[win];
        for (int tab = root->childCount() - 1; tab >= 0; --tab) {
            if (root->child(tab)->checkState(0) == Qt::Unchecked) {
                wd.tabsState.removeAt(tab);
                if (wd.currentTab >= tab) {
                    wd.currentTab--;
                }
            }
        }

        if (wd.tabsState.isEmpty()) {
            data.removeAt(win);
            continue;
        }

        if (wd.currentTab < 0) {
            wd.currentTab = wd.tabsState.size() - 1;
        }
    }

    if (!mApp->restoreStateSlot(p_QupZilla, data)) {
        newSession();
    }
}

void RecoveryWidget::newSession()
{
    m_view->load(p_QupZilla->homepageUrl());

    mApp->destroyRestoreManager();
}

RecoveryWidget::~RecoveryWidget()
{
    delete ui;
}
