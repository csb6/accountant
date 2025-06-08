pragma user_version = 1;

CREATE TABLE stocks (id INTEGER PRIMARY KEY, symbol TEXT UNIQUE, description TEXT);

CREATE TABLE account_kinds (id INTEGER PRIMARY KEY, name TEXT UNIQUE);
INSERT INTO account_kinds(name) VALUES ("Bank");
INSERT INTO account_kinds(name) VALUES ("Income");
INSERT INTO account_kinds(name) VALUES ("Expense");
INSERT INTO account_kinds(name) VALUES ("Stock");

CREATE TABLE accounts (id INTEGER PRIMARY KEY, name TEXT UNIQUE, kind INTEGER REFERENCES account_kinds);
CREATE TABLE account_stocks (id INTEGER PRIMARY KEY REFERENCES accounts, stock_id INTEGER REFERENCES stocks);

CREATE TABLE transactions (id INTEGER PRIMARY KEY, date TEXT, source INTEGER REFERENCES accounts, destination INTEGER REFERENCES accounts, amount INTEGER, CHECK (date IS strftime("%Y-%m-%d", date)));
CREATE TABLE transaction_descriptions (id INTEGER PRIMARY KEY REFERENCES transactions, description TEXT);
