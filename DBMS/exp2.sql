-- Constraints:
-- https://duckdb.org/docs/stable/sql/constraints.html
--
-- Named Constraints:
-- https://stackoverflow.com/questions/1397440/what-is-the-purpose-of-constraint-naming

CREATE TABLE Employee (
  person_name VARCHAR(20) CONSTRAINT Emp_pk PRIMARY KEY,
  street      VARCHAR(20),
  city        VARCHAR(20)
);

CREATE TABLE Company (
  company_name VARCHAR(20) CONSTRAINT Cmp_pk PRIMARY KEY,
  city         VARCHAR(20)
);

CREATE TABLE Works (
  person_name  VARCHAR(20) CONSTRAINT Wrk_pname_fk REFERENCES Employee(person_name),
  company_name VARCHAR(20) CONSTRAINT Wrk_cname_fk REFERENCES Company(company_name),
  salary       USMALLINT,
  CONSTRAINT Wrk_pk PRIMARY KEY (person_name, company_name),
  CONSTRAINT Sal_chk CHECK (salary <= 99999)
);

CREATE TABLE Manages(
  person_name  VARCHAR(20) CONSTRAINT Mng_pk REFERENCES Employee(person_name),
  manager_name VARCHAR(20) CONSTRAINT Mng_name REFERENCES Employee(person_name),
);

INSERT INTO Employee VALUES
  ('John Doe', 'New York Street', 'New York'),
  ('John Dih', 'New York Street',  'New York'),
  ('Jane Doe', 'Yorkshire Street', 'Yorkshire'),
  ('Jane Dih', 'Yorkland Street', 'Yorkland'),
  ('John Jane', 'York DC Street', 'York DC'),
  ('Johnny Jane', 'Paris Street', 'Paris');

INSERT INTO Company VALUES
  ('First Bank Corporation', 'New York'),
  ('Last Bank Limited', 'Paris');

INSERT INTO Works VALUES
  ('John Doe', 'First Bank Corporation', 20000),
  ('John Dih', 'First Bank Corporation', 9000),
  ('Jane Dih', 'Last Bank Limited', 4000),
  ('John Jane', 'First Bank Corporation', 4000),
  ('Johnny Jane', 'Last Bank Limited', 6000);

INSERT INTO Manages VALUES
  ('John Doe', 'John Dih'),
  ('John Jane', 'John Dih'),
  ('Johnny Jane', 'Jane Dih');

SELECT person_name FROM Works
  WHERE company_name == 'First Bank Corporation';

-- JOIN
-- https://duckdb.org/docs/stable/sql/query_syntax/from.html
--
-- INNER JOIN
-- Returns matching rows b/w two tables based on specified cond
--
-- OUTER JOIN
-- Returns all the rows from one table and matching rows from the other table based on specified condition
--  Left Outer Join: Returns all rows of a table on the left side of the Join and for rows for which there is no matching row on the right side, result contains NULL on the right side.
SELECT person_name, city FROM Employee JOIN Works USING (person_name)
  WHERE company_name == 'First Bank Corporation';

SELECT person_name, street, city FROM Employee JOIN Works USING (person_name)
  WHERE company_name == 'First Bank Corporation' AND salary > 10000;

SELECT * FROM Works
  JOIN Company  USING (company_name)
  JOIN Employee USING (person_name);

-- SELECT INTO not supported in duckdb
-- Instead use: CREATE TABLE name AS SELECT ...
CREATE TABLE xoin_table AS
SELECT * FROM Employee
  JOIN Manages USING (person_name);
SELECT xoin_table.person_name FROM xoin_table
  JOIN Employee ON manager_name = Employee.person_name
  WHERE xoin_table.city == Employee.city AND
        xoin_table.street == Employee.street;

-- Name of people who do not work for First Bank Corporation
SELECT person_name FROM Employee
  JOIN Works USING (person_name)
  WHERE Works.company_name != 'First Bank Corporation';

-- GROUP BY w Aggregate Function max
CREATE TABLE sal_table AS
SELECT company_name, max(salary) as max_sal
  FROM Works
  GROUP BY company_name;
SELECT * FROM sal_table;
SELECT person_name FROM Works, sal_table
  WHERE Works.company_name != 'Last Bank Limited'
    AND sal_table.company_name == 'Last Bank Limited'
    AND Works.salary > sal_table.max_sal;
