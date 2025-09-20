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
#include <QTabBar>
#include "models/AccountTree.hpp"
#include "models/Roles.hpp"
#include "views/AccountsView.hpp"
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

    QFileDialog* file_dialog;
    AccountTree* account_tree;
    Ui::MainWindow ui;
};

MainWindow::MainWindow(AccountTree& account_tree)
    : QMainWindow(), m_impl(new Impl(new QFileDialog(this), &account_tree))
{
    m_impl->ui.setupUi(this);
    m_impl->file_dialog->setNameFilter(u"*.db"_s);

    auto* accounts_view = new AccountsView(account_tree);
    connect(accounts_view, &AccountsView::activated, this, &MainWindow::open_transactions_view);
    connect(m_impl->ui.file_open, &QAction::triggered, m_impl->file_dialog, &QDialog::open);
    connect(m_impl->file_dialog, &QDialog::accepted, [this] {
        auto files = m_impl->file_dialog->selectedFiles();
        assert(files.size() == 1);
        if(files.size() == 1) {
            emit database_changed(files[0]);
        }
    });

    m_impl->ui.tabs->addTab(accounts_view, "Accounts");
    m_impl->ui.tabs->setTabToolTip(0, "Accounts");
    // Accounts tab cannot be closed
    auto* tab_bar = m_impl->ui.tabs->tabBar();
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
    for(int i = m_impl->ui.tabs->count() - 1; i > 0; --i) {
        m_impl->close_tab(i);
    }
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
    auto account_transactions = m_impl->account_tree->account_transactions(account);
    if(!account_transactions) {
        return;
    }
    auto* transactions_view = new TransactionsView(std::move(account_transactions));
    auto tab_index = m_impl->ui.tabs->addTab(transactions_view, tab_name);
    m_impl->ui.tabs->setTabToolTip(tab_index, tab_name);
    m_impl->ui.tabs->setCurrentIndex(tab_index);
}
