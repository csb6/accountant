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

#pragma once

enum TransactionsViewColumn {
    TRANSACTIONS_VIEW_ID,
    TRANSACTIONS_VIEW_DATE,
    TRANSACTIONS_VIEW_DESCRIPTION,
    TRANSACTIONS_VIEW_SOURCE,
    TRANSACTIONS_VIEW_DESTINATION,
    TRANSACTIONS_VIEW_UNIT_PRICE,
    TRANSACTIONS_VIEW_QUANTITY,
    TRANSACTIONS_VIEW_AMOUNT,

    TRANSACTIONS_VIEW_COL_COUNT
};

enum AccountsTableColumn {
    ACCOUNTS_ID,
    ACCOUNTS_NAME,
    ACCOUNTS_KIND
};

enum AccountKind {
    ACCOUNT_KIND_BANK = 1,
    ACCOUNT_KIND_INCOME,
    ACCOUNT_KIND_EXPENSE,
    ACCOUNT_KIND_STOCK,
    ACCOUNT_KIND_PLACEHOLDER
};