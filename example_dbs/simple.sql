-- Account kinds:
--   Bank = 1
--   Income = 2
--   Expense = 3
--   Stock = 4

pragma foreign_keys = ON;

INSERT INTO stocks(symbol, description) VALUES ("F", "Ford");
INSERT INTO stocks(symbol, description) VALUES ("GRMN", "Garmin");

INSERT INTO accounts(name, kind) VALUES ("Investment/F", 4);
INSERT INTO account_stocks VALUES (1, 1);

INSERT INTO accounts(name, kind) VALUES ("Investment/GRMN", 4);
INSERT INTO account_stocks VALUES (2, 2);

INSERT INTO accounts(name, kind) VALUES ("Income/Salary", 2);

INSERT INTO transactions(source, date, destination, amount) VALUES (3, "2025-06-07", 2, 1000);
INSERT INTO transaction_descriptions VALUES (1, "Purchase Garmin stock");
