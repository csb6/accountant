/*
QAccountant - accounting program
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
#include "SecurityEditor.hpp"
#include <algorithm>
#include <QErrorMessage>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlTableModel>
#include "models/DatabaseManager.hpp"
#include "models/SQLColumns.hpp"
#include "ui_SecurityEditor.h"

using namespace Qt::StringLiterals;

// TODO: make QErrorMessage variant for common pattern where a QSqlQuery's error message is wrapped (with a default
//   message shown if SQLite doesn't provide an error message)

struct SecurityEditor::Impl {
    explicit
    Impl(QSqlDatabase& db) : db(&db), securities(nullptr, db) {}

    QSqlDatabase* db;
    QSqlTableModel securities;
    Ui::SecurityEditor ui;
};

SecurityEditor::SecurityEditor(QSqlDatabase& db, QWidget* parent)
    : QDialog(parent), m_impl(new Impl(db))
{
    m_impl->ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    m_impl->securities.setTable("securities");
    m_impl->securities.setHeaderData(SECURITIES_SYMBOL, Qt::Horizontal, u"Symbol"_s);
    m_impl->securities.setHeaderData(SECURITIES_DESCRIPTION, Qt::Horizontal, u"Description"_s);
    m_impl->securities.setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_impl->securities.select();
    m_impl->ui.securities_view->setModel(&m_impl->securities);

    connect(m_impl->ui.add_security, &QToolButton::clicked, [this] {
        m_impl->securities.insertRow(m_impl->securities.rowCount());
        // Select the new row (will also autoscroll to that row if not visible)
        m_impl->ui.securities_view->selectRow(m_impl->securities.rowCount() - 1);
    });

    connect(m_impl->ui.delete_security, &QToolButton::clicked, [this] {
        auto selected_items = m_impl->ui.securities_view->selectionModel()->selectedIndexes();
        // Remove starting from highest index to avoid invalidating indices when deleting
        std::ranges::sort(selected_items, [](const auto& a, const auto& b) { return a.row() > b.row(); });
        for(auto item : selected_items) {
            m_impl->ui.securities_view->hideRow(item.row());
            m_impl->securities.removeRow(item.row());
        }
    });

    connect(m_impl->ui.securities_view->selectionModel(), &QItemSelectionModel::selectionChanged, [this] {
        m_impl->ui.delete_security->setEnabled(m_impl->ui.securities_view->selectionModel()->hasSelection());
    });
}

SecurityEditor::~SecurityEditor() noexcept
{
    delete m_impl;
}

void SecurityEditor::accept()
{
    if(m_impl->securities.submitAll()) {
        QDialog::accept();
    } else {
        auto error_msg = m_impl->securities.lastError().text();
        if(error_msg.isEmpty()) {
            error_msg = u"Failed to submit changes"_s;
        }
        auto* error_dialog = new QErrorMessage(this);
        error_dialog->setAttribute(Qt::WA_DeleteOnClose);
        error_dialog->showMessage(error_msg);
    }
}

void SecurityEditor::reject()
{
    if(m_impl->securities.isDirty()) {
        QMessageBox confirmation_popup{this};
        confirmation_popup.setText(u"There are unsaved changes. Do you want to apply or discard them?"_s);
        confirmation_popup.setIcon(QMessageBox::Icon::Question);
        confirmation_popup.setStandardButtons(QMessageBox::Apply | QMessageBox::Discard);
        if(confirmation_popup.exec() == QMessageBox::Apply) {
            accept();
            return;
        }
    }
    QDialog::reject();
}
