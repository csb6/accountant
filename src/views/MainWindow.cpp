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
#include "models/AccountTree.hpp"
#include "TransactionsView.hpp"
#include "ui_mainwindow.h"
#include "models/Roles.hpp"

MainWindow::MainWindow(AccountTree& account_tree)
    : QMainWindow(), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->account_tree_view->setModel(&account_tree);
    auto* tab_bar = ui->tabs->tabBar();
    // Accounts tab cannot be closed
    tab_bar->setTabButton(0, QTabBar::ButtonPosition::RightSide, nullptr);
    tab_bar->setTabButton(0, QTabBar::ButtonPosition::LeftSide, nullptr);

    connect(ui->account_tree_view, &QTreeView::activated, this, &MainWindow::open_transactions_view);
    connect(ui->tabs, &QTabWidget::tabCloseRequested, [this](int tab_index) {
        auto* tab = ui->tabs->widget(tab_index);
        ui->tabs->removeTab(tab_index);
        // Needed since removeTab doesn't delete the object
        delete tab;
    });
}

MainWindow::~MainWindow() noexcept
{
    delete ui;
}

void MainWindow::open_transactions_view(QModelIndex account)
{
    auto tab_name = account.data(Account_Path_Role).toString();
    // Don't add new tab if already open
    for(int i = 0; i < ui->tabs->count(); ++i) {
        if(ui->tabs->tabText(i) == tab_name) {
            ui->tabs->setCurrentIndex(i);
            return;
        }
    }
    auto* account_tree = static_cast<AccountTree*>(ui->account_tree_view->model());
    auto account_model = account_tree->account_at(account);
    if(!account_model) {
        return;
    }
    auto* transactions_view = new TransactionsView(std::move(account_model));
    auto tab_index = ui->tabs->addTab(transactions_view, tab_name);
    ui->tabs->setTabToolTip(tab_index, tab_name);
    ui->tabs->setCurrentIndex(tab_index);
}
