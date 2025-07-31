-- DDL: Data Definition Language
-- DQL: Data Query Language
-- DML: Data Manipulation Language
-- DCL: Data Control Language
-- TCL: Transaction Control Language
-- More on: https://www.geeksforgeeks.org/sql/sql-ddl-dql-dml-dcl-tcl-commands

CREATE TABLE Customer ( 
  First_Name VARCHAR(15),
Last_Name  VARCHAR(15),
  Address    VARCHAR(15),
  City       VARCHAR(15),
  Country    VARCHAR(15),
  Birth_Date DATE
);

INSERT INTO Customer VALUES
  ('John', 'Smith', 'Western Road', 'USA', 'New York', make_date(1969, 12, 12)),
  ('David', 'Stonewall', 'Park Avenue', 'USA', 'San Francisco', make_date(1954, 1, 3)),
  ('Susan', 'Grant', 'Lord Park', 'USA', 'Los Angeles', make_date(1970, 3, 3)),
  ('Paul', 'ONeil', 'Red Cross', 'USA', 'New York', make_date(1982, 9, 17)),
  ('Stephen', 'Grant', 'Carpet Road', 'USA', 'Los Angeles', make_date(1974, 3, 3))
;

ALTER TABLE Customer ADD Gender VARCHAR(1);
ALTER TABLE Customer ADD Email VARCHAR(20);
ALTER TABLE Customer ADD TELEPHONE TEXT;
ALTER TABLE Customer ALTER Address TYPE VARCHAR(80);

UPDATE Customer SET Gender = 'M';
