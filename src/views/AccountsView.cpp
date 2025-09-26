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
#include <QErrorMessage>
#include "models/AccountTree.hpp"
#include "ui_accountsview.h"
#include "util/sql_helpers.hpp"

using namespace Qt::StringLiterals;

struct AccountsView::Impl {
    void update_button_statuses(const QModelIndex& selected_index)
    {
        auto* item = account_tree->itemFromIndex(selected_index);
        ui.add_account->setEnabled(item->hasChildren());
        ui.delete_account->setEnabled(!item->hasChildren() && !selected_index.data().toString().isEmpty());
    }

    AccountTree* account_tree;
    Ui::AccountsView ui;
};

AccountsView::AccountsView(AccountTree& account_tree)
    : m_impl(new Impl(&account_tree))
{
    m_impl->ui.setupUi(this);
    m_impl->ui.tree_view->setModel(&account_tree);
    connect(m_impl->ui.tree_view->itemDelegate(), &QAbstractItemDelegate::closeEditor, [this] {
        auto selected_items = m_impl->ui.tree_view->selectionModel()->selectedIndexes();
        if(selected_items.size() != 1) {
            // Can only add under single parent at a time
            return;
        }
        auto index = selected_items[0];
        if(index.data().toString().isEmpty()) {
            // Remove the item if no text was put into it
            m_impl->account_tree->removeRow(index.row(), index.parent());
        } else {
            m_impl->update_button_statuses(index);
        }
    });

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

    connect(m_impl->ui.delete_account, &QToolButton::clicked, [this] {
        auto selected_items = m_impl->ui.tree_view->selectionModel()->selectedIndexes();
        if(selected_items.size() != 1) {
            // Can only add under single parent at a time
            return;
        }
        auto index = selected_items[0];
        auto* item = m_impl->account_tree->itemFromIndex(index);
        if(item->hasChildren()) {
            // TODO: have some way to distinguish between placeholder accounts versus real accounts
            return;
        }
        try {
            m_impl->account_tree->removeRow(index.row(), index.parent());
        } catch(const sql_helpers::Error&) {
            auto* error_modal = new QErrorMessage(this);
            error_modal->setModal(true);
            error_modal->setAttribute(Qt::WA_DeleteOnClose);
            error_modal->showMessage(u"Failed to delete account (check that no transactions reference this account)"_s);
        }
    });

    connect(m_impl->ui.tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, [this] {
        if(m_impl->ui.tree_view->selectionModel()->hasSelection()) {
            // Tree view is guaranteed to have exactly 1 item selected
            auto index = m_impl->ui.tree_view->selectionModel()->selectedIndexes()[0];
            m_impl->update_button_statuses(index);
        } else {
            m_impl->ui.add_account->setEnabled(false);
            m_impl->ui.delete_account->setEnabled(false);
        }
    });
    connect(m_impl->ui.tree_view, &QAbstractItemView::activated, this, &AccountsView::activated);
}

AccountsView::~AccountsView() noexcept
{
    delete m_impl;
}
