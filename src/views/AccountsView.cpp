/*
QAccountant - personal accounting software
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
#include "models/DatabaseManager.hpp"
#include "models/Roles.hpp"
#include "views/NewAccountDialog.hpp"
#include "ui_accountsview.h"
#include "util/sql_helpers.hpp"

using namespace Qt::StringLiterals;

struct AccountsView::Impl {
    void update_button_statuses(const QModelIndex& selected_index)
    {
        auto* item = account_tree->itemFromIndex(selected_index);
        bool is_placeholder = item->data(Account_Kind_Role).toInt() == ACCOUNT_KIND_PLACEHOLDER;
        ui.add_account->setEnabled(is_placeholder);
        ui.delete_account->setEnabled(!item->hasChildren() && selected_index.parent() != QModelIndex{});
    }

    AccountTree* account_tree;
    DatabaseManager* db_manager;
    Ui::AccountsView ui;
};

AccountsView::AccountsView(AccountTree& account_tree, DatabaseManager& db_manager)
    : m_impl(new Impl(&account_tree, &db_manager))
{
    m_impl->ui.setupUi(this);
    m_impl->ui.tree_view->setModel(&account_tree);

    connect(m_impl->ui.add_account, &QToolButton::clicked, [this] {
        auto selected_items = m_impl->ui.tree_view->selectionModel()->selectedIndexes();
        if(selected_items.size() != 1) {
            // Can only add under single parent at a time
            return;
        }
        auto parent = selected_items[0];
        auto* dialog = new NewAccountDialog(*m_impl->account_tree, *m_impl->db_manager, parent, this);
        connect(dialog, &NewAccountDialog::account_created, [this](const QModelIndex& parent_account, const AccountFields& account_fields) {
            auto new_item = m_impl->account_tree->appendRow(account_fields, parent_account);
            m_impl->ui.tree_view->setCurrentIndex(new_item);
            m_impl->update_button_statuses(new_item);
        });
        dialog->show();
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
            return;
        }
        try {
            m_impl->account_tree->removeRow(index.row(), index.parent());
            // Needed because QItemSelectionModel::selectionChanged occurs before removeRow fully
            // completes, so the child item hasn't been deleted from its parent yet
            auto new_selected_item = m_impl->ui.tree_view->selectionModel()->selectedIndexes()[0];
            m_impl->update_button_statuses(new_selected_item);
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

    connect(m_impl->ui.tree_view, &QAbstractItemView::activated, [this](const QModelIndex& index) {
        if(index.data(Account_Kind_Role) != ACCOUNT_KIND_PLACEHOLDER) {
            emit activated(index);
        }
    });
}

AccountsView::~AccountsView() noexcept
{
    delete m_impl;
}
