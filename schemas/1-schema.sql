pragma user_version = 1;

CREATE TABLE stocks (
    id INTEGER PRIMARY KEY,
    symbol TEXT UNIQUE,
    description TEXT
);

CREATE TABLE account_kinds (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE
);
INSERT INTO account_kinds VALUES (1, "Bank");
INSERT INTO account_kinds VALUES (2, "Income");
INSERT INTO account_kinds VALUES (3, "Expense");
INSERT INTO account_kinds VALUES (4, "Stock");

CREATE TABLE accounts (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE,
    kind INTEGER REFERENCES account_kinds
);
CREATE TABLE account_stocks (
    id INTEGER PRIMARY KEY REFERENCES accounts,
    stock_id INTEGER REFERENCES stocks
);

CREATE TABLE transactions (
    id INTEGER PRIMARY KEY,
    date TEXT,
    description TEXT,
    source INTEGER REFERENCES accounts,
    destination INTEGER REFERENCES accounts,
    CHECK (source != destination)
);
CREATE TABLE cash_transactions (
    transaction_id INTEGER PRIMARY KEY REFERENCES transactions ON DELETE CASCADE,
    amount REAL -- in dollars
);
CREATE TABLE security_transactions (
    transaction_id INTEGER PRIMARY KEY REFERENCES transactions ON DELETE CASCADE,
    unit_price REAL, -- in dollars
    quantity REAL -- in number of shares
);

CREATE VIEW transactions_view (id, date, description, source, destination, unit_price, quantity, amount) AS
    SELECT t.id, t.date, t.description, t.source, t.destination, st.unit_price, st.quantity, ct.amount
    FROM transactions t
    FULL JOIN cash_transactions ct ON ct.transaction_id = t.id
    FULL JOIN security_transactions st ON st.transaction_id = t.id;
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
