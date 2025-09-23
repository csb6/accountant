pragma user_version = 1;

CREATE TABLE stocks (
    id INTEGER PRIMARY KEY,
    symbol TEXT UNIQUE NOT NULL,
    description TEXT NOT NULL
) STRICT;

CREATE TABLE account_kinds (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE NOT NULL
) STRICT;

INSERT INTO account_kinds VALUES (1, "Bank");

INSERT INTO account_kinds VALUES (2, "Income");

INSERT INTO account_kinds VALUES (3, "Expense");

INSERT INTO account_kinds VALUES (4, "Stock");

CREATE TABLE accounts (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    kind INTEGER NOT NULL REFERENCES account_kinds
) STRICT;

CREATE TABLE account_stocks (
    id INTEGER PRIMARY KEY REFERENCES accounts ON DELETE CASCADE,
    stock_id INTEGER NOT NULL REFERENCES stocks
) STRICT;

CREATE TABLE transactions (
    id INTEGER PRIMARY KEY,
    date TEXT NOT NULL,
    description TEXT NOT NULL,
    source INTEGER NOT NULL REFERENCES accounts ON DELETE RESTRICT,
    destination INTEGER NOT NULL REFERENCES accounts ON DELETE RESTRICT,
    CHECK (source != destination)
) STRICT;

CREATE TABLE cash_transactions (
    transaction_id INTEGER PRIMARY KEY REFERENCES transactions ON DELETE CASCADE,
    amount REAL NOT NULL -- in dollars
) STRICT;

CREATE TABLE security_transactions (
    transaction_id INTEGER PRIMARY KEY REFERENCES transactions ON DELETE CASCADE,
    unit_price REAL NOT NULL, -- in dollars
    quantity REAL NOT NULL -- in number of shares
) STRICT;

CREATE VIEW transactions_view (id, date, description, source, destination, unit_price, quantity, amount) AS
    SELECT t.id, t.date, t.description, t.source, t.destination, st.unit_price, st.quantity, ct.amount
    FROM transactions t
    FULL JOIN cash_transactions ct ON ct.transaction_id = t.id
    FULL JOIN security_transactions st ON st.transaction_id = t.id;

CREATE TRIGGER transactions_view_date_add_cash
INSTEAD OF INSERT ON transactions_view
WHEN NEW.amount IS NOT NULL AND NEW.unit_price IS NULL AND NEW.quantity IS NULL
BEGIN
    INSERT INTO transactions(date, description, source, destination)
        VALUES (NEW.date, NEW.description, NEW.source, NEW.destination);
    INSERT INTO cash_transactions VALUES (last_insert_rowid(), NEW.amount);
END;

CREATE TRIGGER transactions_view_date_add_stock
INSTEAD OF INSERT ON transactions_view
WHEN NEW.amount IS NULL AND NEW.unit_price IS NOT NULL AND NEW.quantity IS NOT NULL
BEGIN
    INSERT INTO transactions(date, description, source, destination)
        VALUES (NEW.date, NEW.description, NEW.source, NEW.destination);
    INSERT INTO security_transactions VALUES (last_insert_rowid(), NEW.unit_price, NEW.quantity);
END;

CREATE TRIGGER transactions_view_invalid_add
INSTEAD OF INSERT ON transactions_view
WHEN (NEW.amount IS NEW.unit_price) OR (NEW.amount IS NEW.quantity)
BEGIN
    SELECT RAISE(ABORT, "Transactions cannot have both cash-specific and security-specific fields");
END;

CREATE TRIGGER transactions_view_date_update
INSTEAD OF UPDATE OF date ON transactions_view
BEGIN
    UPDATE transactions SET date = NEW.date WHERE id = NEW.id;
END;

CREATE TRIGGER transactions_view_description_update
INSTEAD OF UPDATE OF description ON transactions_view
BEGIN
    UPDATE transactions SET description = NEW.description WHERE id = NEW.id;
END;

CREATE TRIGGER transactions_view_source_update
INSTEAD OF UPDATE OF source ON transactions_view
BEGIN
    UPDATE transactions SET source = NEW.source WHERE id = NEW.id;
END;

CREATE TRIGGER transactions_view_destination_update
INSTEAD OF UPDATE OF destination ON transactions_view
BEGIN
    UPDATE transactions SET destination = NEW.destination WHERE id = NEW.id;
END;

CREATE TRIGGER transactions_view_unit_price_update
INSTEAD OF UPDATE OF unit_price ON transactions_view
BEGIN
    UPDATE security_transactions SET unit_price = NEW.unit_price WHERE transaction_id = NEW.id;
END;

CREATE TRIGGER transactions_view_quantity_update
INSTEAD OF UPDATE OF quantity ON transactions_view
BEGIN
    UPDATE security_transactions SET quantity = NEW.quantity WHERE transaction_id = NEW.id;
END;

CREATE TRIGGER transactions_view_amount_update
INSTEAD OF UPDATE OF amount ON transactions_view
BEGIN
    UPDATE cash_transactions SET amount = NEW.amount WHERE transaction_id = NEW.id;
END;

CREATE TRIGGER transactions_view_delete
INSTEAD OF DELETE ON transactions_view
BEGIN
    DELETE FROM transactions WHERE id = OLD.id;
END;
