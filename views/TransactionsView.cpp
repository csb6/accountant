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
#include "models/AccountModel.hpp"
#include "models/SQLColumns.hpp"
#include "ui_transactionsview.h"

struct TransactionsView::Impl {
    std::unique_ptr<AccountModel> account;
    Ui::TransactionsView ui;
};

TransactionsView::TransactionsView(std::unique_ptr<AccountModel> account)
    : QFrame(), m_impl(new Impl(std::move(account)))
{
    m_impl->ui.setupUi(this);
    m_impl->ui.transactions_view->setModel(m_impl->account.get());
    m_impl->ui.transactions_view->hideColumn(TRANSACTIONS_ID);
    m_impl->ui.transactions_view->verticalHeader()->setVisible(false);
}

TransactionsView::~TransactionsView() noexcept
{
    delete m_impl;
}
