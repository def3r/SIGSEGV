-- Neovim Auto Inc: g C-a
-- PRIMARY KEY constraint enforeces the keys to be NOT NULL

-- FOREGIN KEY define a col, or set of cols, that refer to a primary key or unique constraint from another table
-- In duckdb FOREIGN KEY is REFERENCES

CREATE TABLE Salesman (
  salesman_id UINTEGER    PRIMARY KEY,
  name        VARCHAR(20) NOT NULL,
  city        VARCHAR(20),
  commission  UINTEGER    NOT NULL
);

CREATE TABLE Customer (
  customer_id UINTEGER    PRIMARY KEY,
  cust_name   VARCHAR(20) NOT NULL,
  city        VARCHAR(20),
  grade       CHAR(1)     NOT NULL,
  salesman_id UINTEGER    REFERENCES Salesman(salesman_id)
);

CREATE TABLE Orders (
  ord_no      UINTEGER PRIMARY KEY,
  purch_amt   UINTEGER NOT NULL,
  ord_date    DATE     NOT NULL,
  customer_id UINTEGER REFERENCES Customer(customer_id),
  salesman_id UINTEGER REFERENCES Salesman(salesman_id)
);

INSERT INTO Salesman
VALUES
(1, 'John Doe', 'New York', 500),
(2, 'John Dih', 'York', 300),
(3, 'Jane Doe', 'Yorkshire', 5000),
(4, 'Jane Dih', 'Yorkland', 100),
(5, 'John Jane', 'York DC', 50),
(6, 'Johnny Jane', 'Paris', 40);

INSERT INTO Customer
VALUES
(1, 'Lorem Ipsum', 'Florence', 'A', 1),
(2, 'Lorem Williams', 'Flor', 'B', 1),
(3, 'Lorem Ford', 'ence', 'C', 3),
(4, 'Ipsum Jordan', 'lorence', 'D', 4),
(5, 'Ipsum Dave', 'Foece', 'oh hell nah', 2);

INSERT INTO Orders
VALUES 
(1, 200, make_date(1789, 7, 14), 1, 1),
(2, 300, make_date(1889, 7, 14), 3, 3),
(3, 400, make_date(1989, 7, 14), 5, 2),
(4, 500, make_date(2089, 7, 14), 2, 1),
(5, 600, make_date(2189, 7, 14), 5, 1);

SELECT name, city FROM Salesman WHERE city == 'Paris';
SELECT salesman_id, name FROM Salesman WHERE commission >= 50;
