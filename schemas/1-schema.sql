pragma user_version = 1;

CREATE TABLE securities (
    symbol TEXT PRIMARY KEY,
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

INSERT INTO account_kinds VALUES (5, "Placeholder");

CREATE TABLE accounts (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    kind INTEGER NOT NULL REFERENCES account_kinds
) STRICT;

INSERT INTO accounts VALUES (1, "Assets", 5);

INSERT INTO accounts VALUES (2, "Equity", 5);

INSERT INTO accounts VALUES (3, "Expenses", 5);

INSERT INTO accounts VALUES (4, "Income", 5);

INSERT INTO accounts VALUES (5, "Liabilities", 5);

CREATE TABLE account_securities (
    id INTEGER PRIMARY KEY REFERENCES accounts ON DELETE CASCADE,
    symbol TEXT NOT NULL REFERENCES securities
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


-- Shows all transactions in terms of cash value
CREATE VIEW transactions_as_cash_view (id, date, description, source, destination, amount) AS
    SELECT t.id, t.date, t.description, t.source, t.destination, iif(ct.amount IS NULL, st.unit_price * st.quantity, ct.amount)
    FROM transactions t
    LEFT JOIN cash_transactions ct ON ct.transaction_id = t.id
    LEFT JOIN security_transactions st ON st.transaction_id = t.id;

CREATE TRIGGER tac_add_row
INSTEAD OF INSERT ON transactions_as_cash_view
BEGIN
    INSERT INTO transactions(date, description, source, destination)
        VALUES (NEW.date, NEW.description, NEW.source, NEW.destination);
    INSERT INTO cash_transactions VALUES (last_insert_rowid(), NEW.amount);
END;

CREATE TRIGGER tac_date_update
INSTEAD OF UPDATE OF date ON transactions_as_cash_view
BEGIN
    UPDATE transactions SET date = NEW.date WHERE id = NEW.id;
END;

CREATE TRIGGER tac_description_update
INSTEAD OF UPDATE OF description ON transactions_as_cash_view
BEGIN
    UPDATE transactions SET description = NEW.description WHERE id = NEW.id;
END;

CREATE TRIGGER tac_source_update
INSTEAD OF UPDATE OF source ON transactions_as_cash_view
BEGIN
    UPDATE transactions SET source = NEW.source WHERE id = NEW.id;
END;

CREATE TRIGGER tac_destination_update
INSTEAD OF UPDATE OF destination ON transactions_as_cash_view
BEGIN
    UPDATE transactions SET destination = NEW.destination WHERE id = NEW.id;
END;

CREATE TRIGGER tac_amount_update
INSTEAD OF UPDATE OF amount ON transactions_as_cash_view
BEGIN
    UPDATE cash_transactions SET amount = NEW.amount WHERE transaction_id = NEW.id;
END;

CREATE TRIGGER tac_delete
INSTEAD OF DELETE ON transactions_as_cash_view
BEGIN
    DELETE FROM transactions WHERE id = OLD.id;
END;


-- Shows all transactions involving securities
CREATE VIEW security_transactions_view (id, date, description, source, destination, unit_price, quantity) AS
    SELECT t.id, t.date, t.description, t.source, t.destination, st.unit_price, st.quantity
    FROM transactions t
    LEFT JOIN security_transactions st ON st.transaction_id = t.id;

CREATE TRIGGER st_add_row
INSTEAD OF INSERT ON security_transactions_view
BEGIN
    INSERT INTO transactions(date, description, source, destination)
        VALUES (NEW.date, NEW.description, NEW.source, NEW.destination);
    INSERT INTO security_transactions VALUES (last_insert_rowid(), NEW.unit_price, NEW.quantity);
END;

CREATE TRIGGER st_date_update
INSTEAD OF UPDATE OF date ON security_transactions_view
BEGIN
    UPDATE transactions SET date = NEW.date WHERE id = NEW.id;
END;

CREATE TRIGGER st_description_update
INSTEAD OF UPDATE OF description ON security_transactions_view
BEGIN
    UPDATE transactions SET description = NEW.description WHERE id = NEW.id;
END;

CREATE TRIGGER st_source_update
INSTEAD OF UPDATE OF source ON security_transactions_view
BEGIN
    UPDATE transactions SET source = NEW.source WHERE id = NEW.id;
END;

CREATE TRIGGER st_destination_update
INSTEAD OF UPDATE OF destination ON security_transactions_view
BEGIN
    UPDATE transactions SET destination = NEW.destination WHERE id = NEW.id;
END;

CREATE TRIGGER st_unit_price_update
INSTEAD OF UPDATE OF unit_price ON security_transactions_view
BEGIN
    UPDATE security_transactions SET unit_price = NEW.unit_price WHERE transaction_id = NEW.id;
END;

CREATE TRIGGER st_quantity_update
INSTEAD OF UPDATE OF quantity ON security_transactions_view
BEGIN
    UPDATE security_transactions SET quantity = NEW.quantity WHERE transaction_id = NEW.id;
END;

CREATE TRIGGER st_delete
INSTEAD OF DELETE ON security_transactions_view
BEGIN
    DELETE FROM transactions WHERE id = OLD.id;
END;
