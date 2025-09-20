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

#include "AccountsView.hpp"
#include "delegates/AccountTreeDelegate.hpp"
#include "models/AccountTree.hpp"
#include "ui_accountsview.h"

struct AccountsView::Impl {

    AccountTree* account_tree;
    Ui::AccountsView ui;
};

AccountsView::AccountsView(AccountTree& account_tree)
    : m_impl(new Impl(&account_tree))
{
    m_impl->ui.setupUi(this);
    m_impl->ui.tree_view->setModel(&account_tree);
    auto* account_tree_delegate = new AccountTreeDelegate(m_impl->ui.tree_view);
    m_impl->ui.tree_view->setItemDelegate(account_tree_delegate);
    connect(account_tree_delegate, &AccountTreeDelegate::editor_closed, &account_tree, &AccountTree::submit_new_item);

    connect(m_impl->ui.add_account, &QToolButton::clicked, [this] {
        auto selected_items = m_impl->ui.tree_view->selectionModel()->selectedIndexes();
        if(selected_items.size() != 1) {
            // Can only add under single parent at a time
            return;
        }
        auto parent = selected_items[0];
        auto* parent_item = m_impl->account_tree->itemFromIndex(parent);
        if(!parent_item->hasChildren()) {
            // TODO: have some way to distinguish between placeholder accounts versus real accounts
            return;
        }
        parent_item->appendRow(new QStandardItem(""));
        auto new_item = parent_item->child(parent_item->rowCount() - 1)->index();
        m_impl->ui.tree_view->setCurrentIndex(new_item);
        m_impl->ui.tree_view->edit(new_item);
    });

    connect(m_impl->ui.tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, [this] {
        if(!m_impl->ui.tree_view->selectionModel()->hasSelection()) {
            m_impl->ui.add_account->setEnabled(false);
        } else {
            // Tree view is guaranteed to have 0 or 1 items selected
            auto parent = m_impl->ui.tree_view->selectionModel()->selectedIndexes()[0];
            auto* parent_item = m_impl->account_tree->itemFromIndex(parent);
            m_impl->ui.add_account->setEnabled(parent_item->hasChildren());
        }
    });
    connect(m_impl->ui.tree_view, &QAbstractItemView::activated, this, &AccountsView::activated);
}

AccountsView::~AccountsView() noexcept
{
    delete m_impl;
}
