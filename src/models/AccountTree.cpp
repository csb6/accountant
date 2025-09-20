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
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QStandardItem>
#include <QString>
#include "Roles.hpp"
#include "SQLColumns.hpp"
#include "util/sql_helpers.hpp"

using namespace Qt::StringLiterals;

struct AccountTree::Impl {
    explicit
    Impl(QSqlDatabase* db) : db(db) {}

    QSqlDatabase* db;
    QSqlQueryModel query_model;
};

static
void build_tree(QSqlQueryModel&, QStandardItem* root);

AccountTree::AccountTree(QSqlDatabase& db)
    : QStandardItemModel(), m_impl(new Impl{&db})
{
    load();
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
    return std::make_unique<AccountTransactions>(*m_impl->db, account_id);
}

void AccountTree::reset()
{
    // Cleans up any query if active so database can be safely closed
    m_impl->query_model.clear();
    clear();
}

void AccountTree::load()
{
    // Database may have changed, so rebuild the query
    m_impl->query_model.setQuery(u"select id, name from accounts order by name"_s, *m_impl->db);
    clear();
    build_tree(m_impl->query_model, invisibleRootItem());
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

void AccountTree::submit_new_item(const QModelIndex& index)
{
    if(index.data().toString().isEmpty()) {
        // Remove the item if no text was put into it
        removeRow(index.row(), index.parent());
    } else {
        auto account_path = index.data(Account_Path_Role).toString();
        QSqlQuery query{*m_impl->db};
        query.prepare("INSERT INTO accounts(name, kind) VALUES (?, ?)");
        query.bindValue(0, account_path);
        // TODO: support other account kinds
        query.bindValue(1, ACCOUNT_KIND_BANK);
        sql_helpers::try_(query, query.exec());
        load();
    }
}

// Assumes query orders the accounts by name (ascending)
static
void build_tree(QSqlQueryModel& query_model, QStandardItem* root)
{
    std::vector<QString> account_name_stack{u""_s};
    std::vector<QStandardItem*> account_stack{root};
    for(int row = 0; row < query_model.rowCount(); ++row) {
        auto account_id = query_model.index(row, ACCOUNTS_ID).data().toInt();
        auto account_path = query_model.index(row, ACCOUNTS_NAME).data().toString();
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
            account_stack.back()->setData(account_id, Account_ID_Role);
        }
    }
}
