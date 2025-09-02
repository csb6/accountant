#include "AccountTreeModel.hpp"
#include <algorithm>
#include <iterator>
#include <vector>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QStandardItem>
#include <QString>
#include "models/Roles.hpp"
#include "models/SQLColumns.hpp"

struct AccountTreeModel::Impl {
    explicit
    Impl(QSqlDatabase* db) : db(db) {}

    QSqlDatabase* db;
    QSqlQueryModel query_model;
};

static
void build_tree(QSqlQueryModel&, QStandardItem* root);

AccountTreeModel::AccountTreeModel(QSqlDatabase& db)
    : QStandardItemModel(), m_impl(new Impl{&db})
{
    m_impl->query_model.setQuery("select id, name from accounts order by name", db);
    build_tree(m_impl->query_model, invisibleRootItem());
}

AccountTreeModel::~AccountTreeModel() noexcept
{
    delete m_impl;
}

std::unique_ptr<AccountModel> AccountTreeModel::account_at(QModelIndex index)
{
    auto* item = itemFromIndex(index);
    auto account_id = item->data(Account_ID_Role).toInt();
    return std::make_unique<AccountModel>(*m_impl->db, account_id);
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
            new_item->setData(account_id, Account_ID_Role);
            account_stack.back()->appendRow(new_item);
            account_stack.push_back(new_item);
            account_name_stack.push_back(*part);
        }
    }
}
