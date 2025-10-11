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

#include "NewAccountDialog.hpp"
#include <QErrorMessage>
#include <QSortFilterProxyModel>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include "models/DatabaseManager.hpp"
#include "models/Roles.hpp"
#include "models/SQLColumns.hpp"
#include "ui_NewAccountDialog.h"

using namespace Qt::StringLiterals;

/* Only show Placeholder accounts (since these are the only accounts that can be parents of other accounts) */
struct PlaceholderAccountTree : public QSortFilterProxyModel {
    using QSortFilterProxyModel::QSortFilterProxyModel;
    ~PlaceholderAccountTree() {}

    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
    {
        auto source_index = sourceModel()->index(source_row, 0, source_parent);
        return source_index.data(Account_Kind_Role).toInt() == ACCOUNT_KIND_PLACEHOLDER;
    }
};

struct NewAccountDialog::Impl {
    Impl(NewAccountDialog* owner, AccountTree& account_tree, DatabaseManager& db_manager, const QModelIndex& initial_parent_index)
        : db_manager(&db_manager)
    {
        ui.setupUi(owner);
        owner->setAttribute(Qt::WA_DeleteOnClose);

        placeholders.setSourceModel(&account_tree);
        ui.parent_accounts_view->setModel(&placeholders);
        ui.parent_accounts_view->setCurrentIndex(placeholders.mapFromSource(initial_parent_index));

        account_kinds.setQuery(u"SELECT id, name FROM account_kinds ORDER BY name"_s, db_manager.database());
        ui.account_kind_view->setModel(&account_kinds);
        ui.account_kind_view->setModelColumn(1);

        connect(ui.account_kind_view, &QComboBox::currentIndexChanged, [this](int i) {
            auto selected_index = account_kinds.index(i, 0);
            update_security_fields(selected_index);
        });

        // Default account kind is not Stock, so hide security-related fields
        ui.symbol_label->setVisible(false);
        ui.symbol_view->setVisible(false);
    }

    void update_security_fields(const QModelIndex& selected_index)
    {
        auto account_kind = selected_index.data().toInt();
        if(account_kind == ACCOUNT_KIND_STOCK) {
            ui.symbol_label->setVisible(true);
            ui.symbol_view->setVisible(true);
            if(!symbols.query().isActive()) {
                symbols.setQuery(u"SELECT symbol FROM securities"_s, db_manager->database());
                ui.symbol_view->setModel(&symbols);
            }
        } else {
            ui.symbol_label->setVisible(false);
            ui.symbol_view->setVisible(false);
        }
    }

    DatabaseManager* db_manager;
    PlaceholderAccountTree placeholders;
    QSqlQueryModel account_kinds;
    QSqlQueryModel symbols;
    Ui::NewAccountDialog ui;
};

NewAccountDialog::NewAccountDialog(AccountTree& account_tree, DatabaseManager& db_manager, const QModelIndex& initial_parent_index, QWidget* parent)
    : QDialog(parent), m_impl(new Impl(this, account_tree, db_manager, initial_parent_index))
{}

NewAccountDialog::~NewAccountDialog() noexcept
{
    delete m_impl;
}

void NewAccountDialog::accept()
{
    if(m_impl->ui.account_name_view->text().isEmpty()) {
        auto* error_dialog = new QErrorMessage(this);
        error_dialog->setAttribute(Qt::WA_DeleteOnClose);
        error_dialog->showMessage(u"Account name not provided"_s);
        return;
    }

    auto account_kind_index = m_impl->ui.account_kind_view->currentIndex();
    auto account_kind = m_impl->account_kinds.index(account_kind_index, 0).data().toInt();

    auto parent_account = m_impl->placeholders.mapToSource(
        m_impl->ui.parent_accounts_view->selectionModel()->selectedIndexes()[0]
    );

    QString symbol;
    if(account_kind == ACCOUNT_KIND_STOCK) {
        symbol = m_impl->ui.symbol_view->currentText();
        if(symbol.isEmpty()) {
            auto* error_dialog = new QErrorMessage(this);
            error_dialog->setAttribute(Qt::WA_DeleteOnClose);
            error_dialog->showMessage(u"Symbol not provided"_s);
            return;
        }
    }

    emit account_created(parent_account, {m_impl->ui.account_name_view->text(), symbol, static_cast<AccountKind>(account_kind)});
    QDialog::accept();
}
