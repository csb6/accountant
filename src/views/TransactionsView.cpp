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
#include <QSqlRelationalDelegate>
#include "models/SQLColumns.hpp"
#include "ui_transactionsview.h"

struct TransactionsView::Impl {
    std::unique_ptr<QSqlRelationalTableModel> transactions;
    QErrorMessage* error_modal;
    Ui::TransactionsView ui;
    std::vector<int> hidden_rows;

    void mark_dirty()
    {
        ui.submit_changes_button->setEnabled(true);
        ui.revert_changes_button->setEnabled(true);
    }

    void reset_undo_state()
    {
        ui.submit_changes_button->setEnabled(false);
        ui.revert_changes_button->setEnabled(false);
        for(auto row : hidden_rows) {
            ui.transactions_view->setRowHidden(row, false);
        }
        hidden_rows.clear();
    }
};

TransactionsView::TransactionsView(std::unique_ptr<QSqlRelationalTableModel> transactions)
    : QFrame(), m_impl(new Impl(std::move(transactions), new QErrorMessage(this)))
{
    m_impl->ui.setupUi(this);
    m_impl->error_modal->setModal(true);
    m_impl->ui.transactions_view->setModel(m_impl->transactions.get());
    m_impl->ui.transactions_view->setItemDelegate(new QSqlRelationalDelegate(m_impl->ui.transactions_view));
    m_impl->ui.transactions_view->hideColumn(TRANSACTIONS_ID);
    m_impl->ui.transactions_view->resizeColumnsToContents();

    connect(m_impl->ui.new_transaction_button, &QToolButton::clicked, [this] {
        m_impl->transactions->insertRow(m_impl->transactions->rowCount());
        m_impl->mark_dirty();
    });

    connect(m_impl->ui.transactions_view->selectionModel(), &QItemSelectionModel::selectionChanged, [this] {
        m_impl->ui.delete_transaction_button->setEnabled(
            m_impl->ui.transactions_view->selectionModel()->hasSelection()
        );
    });

    connect(m_impl->ui.delete_transaction_button, &QToolButton::clicked, [this] {
        auto selected_items = m_impl->ui.transactions_view->selectionModel()->selectedIndexes();
        // Remove starting from highest index to avoid invalidating indices when deleting
        std::ranges::sort(selected_items, std::greater<>());
        for(auto item : selected_items) {
            m_impl->ui.transactions_view->hideRow(item.row());
            m_impl->transactions->removeRow(item.row());
            m_impl->hidden_rows.push_back(item.row());
        }
        m_impl->mark_dirty();
    });

    connect(m_impl->ui.submit_changes_button, &QToolButton::clicked, [this] {
        if(m_impl->transactions->submitAll()) {
            m_impl->reset_undo_state();
            m_impl->ui.delete_transaction_button->setEnabled(false);
        } else {
            auto error_msg = m_impl->transactions->lastError().databaseText();
            if(error_msg.isEmpty()) {
                error_msg = "Failed to submit changes (check that new rows have all fields filled out)";
            }
            m_impl->error_modal->showMessage(error_msg);
        }
    });

    connect(m_impl->ui.revert_changes_button, &QToolButton::clicked, [this] {
        m_impl->transactions->revertAll();
        m_impl->reset_undo_state();
    });

    connect(m_impl->ui.transactions_view->itemDelegate(), &QSqlRelationalDelegate::commitData, [this] {
        // Resize columns whenever a cell is edited (since this could change the width of its column)
        m_impl->ui.transactions_view->resizeColumnsToContents();
        m_impl->ui.submit_changes_button->setEnabled(m_impl->transactions->isDirty());
        m_impl->ui.revert_changes_button->setEnabled(m_impl->transactions->isDirty());
    });
}

TransactionsView::~TransactionsView() noexcept
{
    delete m_impl;
}
