/*
Accountant - accounting program
Copyright (C) 2025  Cole Blakley

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "MainWindow.hpp"
#include <cassert>
#include <QFileDialog>
#include "models/AccountTree.hpp"
#include "models/Roles.hpp"
#include "delegates/AccountTreeDelegate.hpp"
#include "TransactionsView.hpp"
#include "ui_mainwindow.h"

using namespace Qt::StringLiterals;

struct MainWindow::Impl {
    void close_tab(int tab_index)
    {
        auto* tab = ui.tabs->widget(tab_index);
        ui.tabs->removeTab(tab_index);
        // Needed since removeTab doesn't delete the object, and tab would otherwise stick around
        // until ui.tabs is destroyed. Since ui.tabs is only destroyed at program close and
        // tabs are opened/closed frequently, this would effectively be a memory leak. To avoid this,
        // we delete it right away.
        tab->setParent(nullptr);
        delete tab;
    }

    void close_all_transaction_tabs()
    {
        for(int i = ui.tabs->count() - 1; i > 0; --i) {
            close_tab(i);
        }
    }

    QFileDialog* file_dialog;
    Ui::MainWindow ui;
};

MainWindow::MainWindow(AccountTree& account_tree)
    : QMainWindow(), m_impl(new Impl(new QFileDialog(this)))
{
    m_impl->ui.setupUi(this);
    m_impl->file_dialog->setNameFilter(u"*.db"_s);

    m_impl->ui.account_tree_view->setModel(&account_tree);
    auto* account_tree_delegate = new AccountTreeDelegate(m_impl->ui.account_tree_view);
    m_impl->ui.account_tree_view->setItemDelegate(account_tree_delegate);
    connect(m_impl->ui.account_tree_view, &QTreeView::activated, this, &MainWindow::open_transactions_view);
    connect(m_impl->ui.file_open, &QAction::triggered, m_impl->file_dialog, &QDialog::open);
    connect(m_impl->file_dialog, &QDialog::accepted, [this] {
        auto files = m_impl->file_dialog->selectedFiles();
        assert(files.size() == 1);
        if(files.size() == 1) {
            emit database_changed(files[0]);
        }
    });
    connect(m_impl->ui.add_account, &QToolButton::clicked, [this] {
        auto selected_items = m_impl->ui.account_tree_view->selectionModel()->selectedIndexes();
        if(selected_items.size() != 1) {
            // Can only add under single parent at a time
            return;
        }
        auto parent = selected_items[0];
        auto* account_tree = static_cast<AccountTree*>(m_impl->ui.account_tree_view->model());
        auto* parent_item = account_tree->itemFromIndex(parent);
        if(!parent_item->hasChildren()) {
            // TODO: have some way to distinguish between placeholder accounts versus real accounts
            return;
        }
        parent_item->appendRow(new QStandardItem(""));
        auto new_item = parent_item->child(parent_item->rowCount() - 1)->index();
        m_impl->ui.account_tree_view->setCurrentIndex(new_item);
        m_impl->ui.account_tree_view->edit(new_item);
    });
    connect(m_impl->ui.account_tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, [this] {
        if(!m_impl->ui.account_tree_view->selectionModel()->hasSelection()) {
            m_impl->ui.add_account->setEnabled(false);
        } else {
            auto* account_tree = static_cast<AccountTree*>(m_impl->ui.account_tree_view->model());
            // Tree view is guaranteed to have 0 or 1 items selected
            auto parent = m_impl->ui.account_tree_view->selectionModel()->selectedIndexes()[0];
            auto* parent_item = account_tree->itemFromIndex(parent);
            m_impl->ui.add_account->setEnabled(parent_item->hasChildren());
        }
    });
    connect(account_tree_delegate, &AccountTreeDelegate::editor_closed, &account_tree, &AccountTree::submit_new_item);

    auto* tab_bar = m_impl->ui.tabs->tabBar();
    // Accounts tab cannot be closed
    tab_bar->setTabButton(0, QTabBar::ButtonPosition::RightSide, nullptr);
    tab_bar->setTabButton(0, QTabBar::ButtonPosition::LeftSide, nullptr);
    connect(m_impl->ui.tabs, &QTabWidget::tabCloseRequested, [this](int tab_index) {
        m_impl->close_tab(tab_index);
    });
}

MainWindow::~MainWindow() noexcept
{
    delete m_impl;
}

void MainWindow::reset()
{
    m_impl->close_all_transaction_tabs();
}

void MainWindow::open_transactions_view(QModelIndex account)
{
    auto tab_name = account.data(Account_Path_Role).toString();
    // Don't add new tab if already open
    for(int i = 0; i < m_impl->ui.tabs->count(); ++i) {
        if(m_impl->ui.tabs->tabText(i) == tab_name) {
            m_impl->ui.tabs->setCurrentIndex(i);
            return;
        }
    }
    auto* account_tree = static_cast<AccountTree*>(m_impl->ui.account_tree_view->model());
    auto account_transactions = account_tree->account_transactions(account);
    if(!account_transactions) {
        return;
    }
    auto* transactions_view = new TransactionsView(std::move(account_transactions));
    auto tab_index = m_impl->ui.tabs->addTab(transactions_view, tab_name);
    m_impl->ui.tabs->setTabToolTip(tab_index, tab_name);
    m_impl->ui.tabs->setCurrentIndex(tab_index);
}
