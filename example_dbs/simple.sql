-- Account kinds:
--   Bank = 1
--   Income = 2
--   Expense = 3
--   Stock = 4
--   Placeholder = 5

pragma foreign_keys = ON;

-- Securities
INSERT INTO stocks VALUES (1, "F", "Ford");
INSERT INTO stocks VALUES (2, "GRMN", "Garmin");

-- Accounts
INSERT INTO accounts VALUES (1, "Investment:F", 4);
INSERT INTO account_stocks VALUES (1, 1);
INSERT INTO accounts VALUES (2, "Investment:GRMN", 4);
INSERT INTO account_stocks VALUES (2, 2);
INSERT INTO accounts VALUES (3, "Income:Salary", 2);
INSERT INTO accounts VALUES (4, "Assets:Checking", 1);
INSERT INTO accounts VALUES (5, "Assets", 5);
INSERT INTO accounts VALUES (6, "Income", 5);
INSERT INTO accounts VALUES (7, "Investment", 5);

-- Transactions
INSERT INTO transactions VALUES (1, "2025-06-10", "Paycheck", 3, 4);
INSERT INTO cash_transactions VALUES (1, 1012.56);

INSERT INTO transactions VALUES (2, "2025-06-07", "Purchase Garmin stock", 4, 2);
INSERT INTO security_transactions VALUES (2, 11.50, 7);

INSERT INTO transactions VALUES (3, "2025-06-08", "Purchase Ford stock", 4, 1);
INSERT INTO security_transactions VALUES (3, 5.078, 19);

INSERT INTO transactions VALUES (4, "2025-07-01", "Sell Garmin stock", 4, 2);
INSERT INTO security_transactions VALUES (4, 10.0001, -2);
