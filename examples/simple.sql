pragma foreign_keys = ON;

-- Securities
INSERT INTO securities VALUES ("F", "Ford");
INSERT INTO securities VALUES ("GRMN", "Garmin");

-- Accounts
INSERT INTO accounts VALUES (6, "Assets:Checking", unicode('B'));
INSERT INTO accounts VALUES (7, "Assets:Savings", unicode('B'));
INSERT INTO accounts VALUES (8, "Assets:Investment", unicode('P'));
INSERT INTO accounts VALUES (9, "Assets:Investment:F", unicode('S'));
INSERT INTO account_securities VALUES (9, "F");
INSERT INTO accounts VALUES (10, "Assets:Investment:GRMN", unicode('S'));
INSERT INTO account_securities VALUES (10, "GRMN");
INSERT INTO accounts VALUES (11, "Income:Salary", unicode('I'));

-- Transactions
INSERT INTO transactions VALUES (1, "2025-06-10", "Paycheck", 11, 6);
INSERT INTO cash_transactions VALUES (1, 1012.56);

INSERT INTO transactions VALUES (2, "2025-06-07", "Purchase Garmin stock", 6, 10);
INSERT INTO security_transactions VALUES (2, 11.50, 7);

INSERT INTO transactions VALUES (3, "2025-06-08", "Purchase Ford stock", 6, 9);
INSERT INTO security_transactions VALUES (3, 5.078, 19);

INSERT INTO transactions VALUES (4, "2025-07-01", "Sell Garmin stock", 10, 6);
INSERT INTO security_transactions VALUES (4, 10.0001, -2);
