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

#include "TransactionsView.hpp"
#include <algorithm>
#include <vector>
#include <QComboBox>
#include <QErrorMessage>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QSqlQueryModel>
#include <QSqlError>
#include "models/SQLColumns.hpp"
#include "ui_transactionsview.h"

using namespace Qt::StringLiterals;

static const QString relation_query_text = u"SELECT id, name FROM accounts ORDER BY name"_s;

/* Maps account IDs into account names for display. Also provides a combo box editor for
   selecting an account name */
struct AccountRelationDelegate : public QStyledItemDelegate {
    enum {
        ID_COLUMN,
        NAME_COLUMN
    };

    explicit
    AccountRelationDelegate(const QSqlDatabase& db, QWidget* parent = nullptr)
        : QStyledItemDelegate(parent)
    {
        m_account_names.setQuery(relation_query_text, db);
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override;
    void setEditorData(QWidget* editor, const QModelIndex&) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem&, const QModelIndex&) const override;
    QString displayText(const QVariant&, const QLocale&) const override;
    void setModelData(QWidget* editor, QAbstractItemModel*, const QModelIndex&) const override;
private:
    QModelIndex find_account_name_index(int account_id) const;

    QSqlQueryModel m_account_names;
};

struct DefaultDelegate : public QStyledItemDelegate {
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        /* Disallows edits on cells containing null data (except if the row is new and hasn't been submitted yet) */
        if(index.data().isNull() && !index.siblingAtColumn(TRANSACTIONS_VIEW_ID).data().isNull()) {
            return nullptr;
        }
        auto* editor = QStyledItemDelegate::createEditor(parent, option, index);
        if(auto* spinbox = qobject_cast<QDoubleSpinBox*>(editor)) {
            spinbox->setDecimals(4);
        }
        return editor;
    }
};

struct TransactionsView::Impl {
    Impl(TransactionsView* owner, std::unique_ptr<QSqlTableModel> transactions)
        : m_transactions(std::move(transactions)), m_error_modal(new QErrorMessage(owner))
    {
        m_ui.setupUi(owner);
        m_error_modal->setModal(true);
        m_ui.transactions_view->setModel(m_transactions.get());
        m_ui.transactions_view->hideColumn(TRANSACTIONS_VIEW_ID);

        connect(m_ui.new_transaction, &QToolButton::clicked, [this] {
            m_transactions->insertRow(m_transactions->rowCount());
            set_dirty();
            // Select the new row (will also autoscroll to that row if not visible)
            m_ui.transactions_view->selectRow(m_transactions->rowCount() - 1);
        });

        connect(m_ui.transactions_view->selectionModel(), &QItemSelectionModel::selectionChanged, [this] {
            m_ui.delete_transaction->setEnabled(
                m_ui.transactions_view->selectionModel()->hasSelection()
            );
        });

        connect(m_ui.delete_transaction, &QToolButton::clicked, [this] {
            auto selected_items = m_ui.transactions_view->selectionModel()->selectedIndexes();
            // Remove starting from highest index to avoid invalidating indices when deleting
            std::ranges::sort(selected_items, [](const auto& a, const auto& b) { return a.row() > b.row(); });
            for(auto item : selected_items) {
                m_ui.transactions_view->hideRow(item.row());
                m_transactions->removeRow(item.row());
                m_hidden_rows.push_back(item.row());
            }
            set_dirty();
        });

        connect(m_ui.submit_changes, &QToolButton::clicked, [this] {
            if(m_transactions->submitAll()) {
                clear_pending_changes();
                // Note: Submitting the changes causes the view to be refreshed, after
                //  which no row will be selected. This means the delete_transaction
                //  button should be greyed out
                m_ui.delete_transaction->setEnabled(false);
            } else {
                auto error_msg = m_transactions->lastError().text();
                if(error_msg.isEmpty()) {
                    error_msg = u"Failed to submit changes (check that new rows have all fields filled out)"_s;
                }
                m_error_modal->showMessage(error_msg);
            }
        });

        connect(m_ui.revert_changes, &QToolButton::clicked, [this] {
            m_transactions->revertAll();
            clear_pending_changes();
        });

        auto* default_delegate = new DefaultDelegate(owner);
        m_ui.transactions_view->setItemDelegate(default_delegate);
        auto* account_relation_delegate = new AccountRelationDelegate(m_transactions->database(), owner);
        m_ui.transactions_view->setItemDelegateForColumn(TRANSACTIONS_VIEW_DESTINATION, account_relation_delegate);
        m_ui.transactions_view->setItemDelegateForColumn(TRANSACTIONS_VIEW_SOURCE, account_relation_delegate);

        // Resize columns whenever a cell is edited (since this could change the width of its column)
        auto on_commit = [this] {
            m_ui.transactions_view->resizeColumnsToContents();
            set_dirty(m_transactions->isDirty());
        };
        connect(m_ui.transactions_view->itemDelegate(), &QAbstractItemDelegate::commitData, on_commit);
        connect(account_relation_delegate, &AccountRelationDelegate::commitData, on_commit);
        m_ui.transactions_view->resizeColumnsToContents();
    }

    void set_dirty(bool value = true)
    {
        m_ui.submit_changes->setEnabled(value);
        m_ui.revert_changes->setEnabled(value);
    }

    void clear_pending_changes()
    {
        set_dirty(false);
        for(auto row : m_hidden_rows) {
            m_ui.transactions_view->setRowHidden(row, false);
        }
        m_hidden_rows.clear();
        m_ui.transactions_view->resizeColumnsToContents();
    }

    std::unique_ptr<QSqlTableModel> m_transactions;
    QErrorMessage* m_error_modal;
    Ui::TransactionsView m_ui;
    std::vector<int> m_hidden_rows;
};

TransactionsView::TransactionsView(std::unique_ptr<QSqlTableModel> transactions)
    : QFrame(), m_impl(new Impl(this, std::move(transactions)))
{}

TransactionsView::~TransactionsView() noexcept
{
    delete m_impl;
}

QWidget* AccountRelationDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
    auto& self = const_cast<AccountRelationDelegate&>(*this);
    const auto* transactions_model = static_cast<const QSqlTableModel*>(index.model());
    self.m_account_names.setQuery(relation_query_text, transactions_model->database());

    auto* combo_box = new QComboBox(parent);
    combo_box->setModel(&self.m_account_names);
    combo_box->setModelColumn(NAME_COLUMN); // Only show the account name, not the ID
    return combo_box;
}

void AccountRelationDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto account_id = index.data().toInt();
    auto account_name_index = find_account_name_index(account_id);
    auto* combo_box = static_cast<QComboBox*>(editor);
    combo_box->setCurrentIndex(account_name_index.row());
}

void AccountRelationDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const
{
    editor->setGeometry(option.rect);
}

QString AccountRelationDelegate::displayText(const QVariant& value, const QLocale&) const
{
    auto account_id = value.toInt();
    auto account_name_index = find_account_name_index(account_id);
    return account_name_index.siblingAtColumn(NAME_COLUMN).data().toString();
}

void AccountRelationDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* combo_box = static_cast<QComboBox*>(editor);
    int row = combo_box->currentIndex();
    auto account_id = combo_box->model()->index(row, ID_COLUMN).data().toInt();
    model->setData(index, account_id);
}

// TODO: this is a linear scan of the query results - might be too slow for large numbers of accounts
QModelIndex AccountRelationDelegate::find_account_name_index(int account_id) const
{
    int row_count = m_account_names.rowCount();
    for(int row = 0; row < row_count; ++row) {
        auto i = m_account_names.index(row, ID_COLUMN);
        if(i.data().toInt() == account_id) {
            return i;
        }
    }
    return {};
}
