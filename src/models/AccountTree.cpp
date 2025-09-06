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
#include <iterator>
#include <vector>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QStandardItem>
#include <QString>
#include "Roles.hpp"
#include "SQLColumns.hpp"

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
    m_impl->query_model.setQuery("select id, name from accounts order by name", db);
    build_tree(m_impl->query_model, invisibleRootItem());
}

AccountTree::~AccountTree() noexcept
{
    delete m_impl;
}

std::unique_ptr<AccountTransactions> AccountTree::account_transactions(QModelIndex index)
{
    auto* item = itemFromIndex(index);
    if(item->hasChildren()) {
        // Parent accounts don't have transactions of their own
        return {};
    }
    auto account_id = item->data(Account_ID_Role).toInt();
    return std::make_unique<AccountTransactions>(*m_impl->db, account_id);
}

static
void build_tree(QSqlQueryModel& query_model, QStandardItem* root)
{
    std::vector<QString> account_name_stack;
    std::vector<QStandardItem*> account_stack{root};
    for(int row = 0; row < query_model.rowCount(); ++row) {
        auto account_id = query_model.index(row, ACCOUNTS_ID).data().toInt();
        auto account_path = query_model.index(row, ACCOUNTS_NAME).data().toString();
        auto parts = account_path.split('/');
        // Pop items from the stack until the stack is a prefix of parts
        // This means we are at a point in the tree where we can add new parts
        auto[mismatch, first_new_part] = std::ranges::mismatch(account_name_stack, parts);
        size_t delete_count = std::distance(mismatch, account_name_stack.end());
        for(size_t i = 0; i < delete_count; ++i) {
            account_stack.pop_back();
            account_name_stack.pop_back();
        }
        // Add all of the new parts to the tree (and the stack)
        for(auto part = first_new_part; part != parts.end(); ++part) {
            auto* new_item = new QStandardItem(*part);
            account_stack.back()->appendRow(new_item);
            account_stack.push_back(new_item);
            account_name_stack.push_back(*part);
        }
        if(!account_stack.empty()) {
            auto* new_item = account_stack.back();
            new_item->setData(account_id, Account_ID_Role);
            new_item->setData(account_path, Account_Path_Role);
        }
    }
}
