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

#include "AccountTree.hpp"
#include <algorithm>
#include <vector>
#include <QSqlQuery>
#include <QStandardItem>
#include <QString>
#include "Roles.hpp"
#include "util/sql_helpers.hpp"

using namespace Qt::StringLiterals;

struct AccountTree::Impl {
    explicit
    Impl(QSqlDatabase* db) : db(db) {}

    QSqlDatabase* db;
};

static
void build_tree(const QSqlDatabase&, QStandardItem* root);

AccountTree::AccountTree(QSqlDatabase& db)
    : QStandardItemModel(), m_impl(new Impl{&db})
{}

AccountTree::~AccountTree() noexcept
{
    delete m_impl;
}

std::unique_ptr<AccountTransactions> AccountTree::account_transactions(const QModelIndex& index)
{
    auto* item = itemFromIndex(index);
    if(item->hasChildren()) {
        // Parent accounts don't have transactions of their own
        return {};
    }
    auto account_id = item->data(Account_ID_Role).toInt();
    return std::make_unique<AccountTransactions>(*m_impl->db, account_id);
}

void AccountTree::load()
{
    clear();
    build_tree(*m_impl->db, invisibleRootItem());
}

QVariant AccountTree::data(const QModelIndex& index, int role) const
{
    if(role == Account_Path_Role && index.isValid()) {
        auto path = index.data().toString();
        for(auto it = index.parent(); it.isValid(); it = it.parent()) {
            path.prepend("/");
            path.prepend(it.data().toString());
        }
        return path;
    }
    return QStandardItemModel::data(index, role);
}

bool AccountTree::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(role == Qt::EditRole) {
        auto fields = qvariant_cast<AccountFields>(value);
        // Set the item's display data (i.e. the account name). This is needed when
        // building the account path
        QStandardItemModel::setData(index, fields.name, role);
        auto account_path = index.data(Account_Path_Role).toString();
        QSqlQuery query{*m_impl->db};
        sql_helpers::prepare(query, "INSERT INTO accounts(name, kind) VALUES (?, ?) RETURNING id");
        query.bindValue(0, account_path);
        // TODO: support stock fields
        query.bindValue(1, static_cast<int>(fields.kind));
        sql_helpers::exec(query);
        sql_helpers::next(query);
        auto account_id = query.value(0).toInt();
        QStandardItemModel::setData(index, account_id, Account_ID_Role);
        QStandardItemModel::setData(index, static_cast<int>(fields.kind), Account_Kind_Role);
        return true;
    }
    return QStandardItemModel::setData(index, value, role);
}

bool AccountTree::removeRows(int row, int count, const QModelIndex& parent)
{
    QSqlQuery query{*m_impl->db};
    for(int i = 0; i < count; ++i) {
        auto index = this->index(row + i, 0, parent);
        if(!index.data().toString().isEmpty()) {
            auto account_id = index.data(Account_ID_Role).toInt();
            sql_helpers::prepare(query, "DELETE FROM accounts WHERE id = ?");
            query.bindValue(0, account_id);
            sql_helpers::exec(query);
        }
    }
    QStandardItemModel::removeRows(row, count, parent);
    return true;
}

static
void build_tree(const QSqlDatabase& db, QStandardItem* root)
{
    QSqlQuery query{db};
    sql_helpers::exec(query, u"SELECT id, name, kind FROM accounts ORDER BY name"_s);
    std::vector<QString> account_name_stack{u""_s};
    std::vector<QStandardItem*> account_stack{root};
    while(query.next()) {
        auto account_id = query.value(0).toInt();
        auto account_path = query.value(1).toString();
        auto account_kind = query.value(2).toInt();
        auto parts = account_path.split('/');
        // Find the point where the stack and account_path differ.
        // This is the point in the path where new parts need to be added to the tree.
        auto[first_to_delete, first_new_part] = std::mismatch(account_name_stack.begin() + 1, account_name_stack.end(), parts.begin(), parts.end());
        size_t start_pt = first_to_delete - account_name_stack.begin();
        account_stack.erase(account_stack.begin() + start_pt, account_stack.end());
        account_name_stack.erase(account_name_stack.begin() + start_pt, account_name_stack.end());
        // Add the new parts to the tree
        for(auto part = first_new_part; part != parts.end(); ++part) {
            auto* new_item = new QStandardItem(*part);
            account_stack.back()->appendRow(new_item);
            account_stack.push_back(new_item);
            account_name_stack.push_back(*part);
        }
        if(!account_stack.empty()) {
            auto* new_account = account_stack.back();
            new_account->setData(account_id, Account_ID_Role);
            new_account->setData(account_kind, Account_Kind_Role);
        }
    }
}
