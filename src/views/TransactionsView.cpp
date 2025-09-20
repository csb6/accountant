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
#include <QErrorMessage>
#include <QSqlError>
#include "models/SQLColumns.hpp"
#include "ui_transactionsview.h"

using namespace Qt::StringLiterals;

struct TransactionsView::Impl {
    Impl(TransactionsView* owner, std::unique_ptr<QSqlTableModel> transactions)
        : m_transactions(std::move(transactions)), m_error_modal(new QErrorMessage(owner))
    {
        m_ui.setupUi(owner);
        m_error_modal->setModal(true);
        m_ui.transactions_view->setModel(m_transactions.get());
        m_ui.transactions_view->hideColumn(TRANSACTIONS_VIEW_ID);
        m_ui.transactions_view->resizeColumnsToContents();

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
                auto error_msg = m_transactions->lastError().databaseText();
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

        // TODO: have custom delegates that avoid using the generated column name for source/destination and
        //  only update cells that have actually changed

        // Resize columns whenever a cell is edited (since this could change the width of its column)
        connect(m_ui.transactions_view->itemDelegate(), &QAbstractItemDelegate::commitData, [this] {
            m_ui.transactions_view->resizeColumnsToContents();
            set_dirty(m_transactions->isDirty());
        });
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
