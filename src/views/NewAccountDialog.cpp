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

#include "NewAccountDialog.hpp"
#include <QSqlQueryModel>
#include "models/DatabaseManager.hpp"
#include "models/Roles.hpp"
#include "models/SQLColumns.hpp"
#include "ui_NewAccountDialog.h"

using namespace Qt::StringLiterals;

struct NewAccountDialog::Impl {
    AccountTree* account_tree;
    QSqlQueryModel account_types;
    Ui::NewAccountDialog ui;
};

NewAccountDialog::NewAccountDialog(AccountTree& account_tree, DatabaseManager& db_manager, const QModelIndex& initial_parent_index, QWidget* parent)
    : QDialog(parent), m_impl(new Impl(&account_tree))
{
    m_impl->ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    m_impl->ui.parent_accounts->setModel(&account_tree);
    m_impl->ui.parent_accounts->setCurrentIndex(initial_parent_index);

    m_impl->account_types.setQuery(u"SELECT id, name FROM account_kinds ORDER BY name"_s, db_manager.database());
    m_impl->ui.account_type->setModel(&m_impl->account_types);
    m_impl->ui.account_type->setModelColumn(1);

    connect(this, &NewAccountDialog::accepted, [this] {
        auto parent_account = m_impl->ui.parent_accounts->selectionModel()->selectedIndexes()[0];
        if(parent_account.data(Account_Kind_Role).toInt() != ACCOUNT_KIND_PLACEHOLDER) {
            // TODO: better message or disable accounts in treeview
            return;
        }
        auto selected_index = m_impl->ui.account_type->currentIndex();
        auto account_kind = m_impl->account_types.index(selected_index, 0).data().toInt();
        emit account_created(parent_account, {m_impl->ui.account->text(), static_cast<AccountKind>(account_kind)});
    });
}

NewAccountDialog::~NewAccountDialog() noexcept
{
    delete m_impl;
}
