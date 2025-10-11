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

#include "AccountTree.hpp"
#include <algorithm>
#include <vector>
#include <QSqlQuery>
#include <QStandardItem>
#include <QString>
#include "DatabaseManager.hpp"
#include "Roles.hpp"
#include "util/sql_helpers.hpp"

using namespace Qt::StringLiterals;

struct AccountTree::Impl {
    DatabaseManager* db_manager;
};

static
void build_tree(const QSqlDatabase&, QStandardItem* root);

AccountTree::AccountTree(DatabaseManager& db_manager)
    : QStandardItemModel(), m_impl(new Impl{&db_manager})
{
    connect(&db_manager, &DatabaseManager::database_loaded, this, &AccountTree::load);
    connect(&db_manager, &DatabaseManager::database_closing, this, &AccountTree::clear);
}

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
    auto account_kind = static_cast<AccountKind>(item->data(Account_Kind_Role).toInt());
    return std::make_unique<AccountTransactions>(m_impl->db_manager->database(), account_id, account_kind);
}

void AccountTree::load()
{
    clear();
    build_tree(m_impl->db_manager->database(), invisibleRootItem());
}

QVariant AccountTree::data(const QModelIndex& index, int role) const
{
    if(role == Account_Path_Role && index.isValid()) {
        auto path = index.data().toString();
        for(auto it = index.parent(); it.isValid(); it = it.parent()) {
            path.prepend(':').prepend(it.data().toString());
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
        auto account_path = data(index, Account_Path_Role).toString();
        QSqlQuery query{m_impl->db_manager->database()};
        sql_helpers::prepare(query, u"INSERT INTO accounts(name, kind) VALUES (?, ?) RETURNING id"_s);
        query.bindValue(0, account_path);
        query.bindValue(1, static_cast<int>(fields.kind));
        sql_helpers::exec(query);
        sql_helpers::next(query);
        auto account_id = query.value(0).toInt();
        QStandardItemModel::setData(index, account_id, Account_ID_Role);
        QStandardItemModel::setData(index, static_cast<int>(fields.kind), Account_Kind_Role);
        if(fields.kind == ACCOUNT_KIND_STOCK) {
            sql_helpers::prepare(query, u"INSERT INTO account_securities VALUES (?, ?)"_s);
            query.bindValue(0, account_id);
            query.bindValue(1, fields.symbol);
            sql_helpers::exec(query);
        }
        return true;
    }
    return QStandardItemModel::setData(index, value, role);
}

QModelIndex AccountTree::appendRow(const AccountFields& fields, const QModelIndex& parent)
{
    auto* parent_item = itemFromIndex(parent);
    parent_item->appendRow(new QStandardItem(fields.name));
    auto new_item = parent_item->child(parent_item->rowCount() - 1)->index();
    QVariant value;
    value.setValue(fields);
    setData(new_item, value);
    return new_item;
}

bool AccountTree::removeRows(int row, int count, const QModelIndex& parent)
{
    QSqlQuery query{m_impl->db_manager->database()};
    for(int i = 0; i < count; ++i) {
        auto index = this->index(row + i, 0, parent);
        if(!index.data().toString().isEmpty()) {
            auto account_id = index.data(Account_ID_Role).toInt();
            sql_helpers::prepare(query, u"DELETE FROM accounts WHERE id = ?"_s);
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
        auto parts = account_path.split(':');
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
