-- INFO:https://www.w3resource.com/sql-exercises/sql-aggregate-functions.php
-- Questions from above, in order
SELECT
  SUM(purch_amt)
FROM
  orders;

SELECT
  AVG(purch_amt)
FROM
  orders;

-- NOTE:
-- Count number of unique Salesmen
-- Wrong
SELECT
  UNIQUE (salesman_id)
FROM
  orders;

-- Correct:
SELECT
  COUNT(DISTINCT salesman_id)
FROM
  orders;

-- NOTE:
-- Better WAY TO DO THIS:
SELECT
  COUNT(grade)
FROM
  customer
WHERE
  grade != NULL;

-- IS THIS:
-- ALL includes those rows where the mentioned col is NOT NULL
SELECT
  COUNT(ALL.grade) -- this is ALL grade and not ALL.grade
FROM
  customer;

SELECT
  MAX(purch_amt)
FROM
  orders;

SELECT
  MIN(purch_amt)
FROM
  orders;

-- NOTE: GROUP BY
SELECT
  city,
  MAX(grade)
FROM
  customer
GROUP BY
  city;

SELECT
  customer_id,
  MAX(purch_amt)
FROM
  orders
GROUP BY
  customer_id;

-- BUG: Incorrect:
-- Every Non-aggregate col in SELECT query should be mentioned
-- in GROUP BY
--
-- SELECT
--   customer_id,
--   ord_date,
--   MAX(purch_amt)
-- FROM
--   orders
-- GROUP BY
--   ord_date -- missing customer_id col
-- HAVING
--   purch_amt = MAX(purch_amt);
SELECT
  customer_id,
  ord_date,
  MAX(purch_amt)
FROM
  orders
GROUP BY -- GROUP BY includes every unique pair of (customer_id, ord_date)
  customer_id,
  ord_date;

-- NOTE: NOT GOOD SOL
SELECT
  salesman_id,
  MAX(purch_amt)
FROM
  (
    SELECT
      *
    FROM
      orders
    WHERE
      ord_date = '2012-08-17'
  );

-- Better sol
SELECT
  salesman_id,
  MAX(purch_amt)
FROM
  orders
WHERE
  ord_date = '2012-08-17'
GROUP BY
  salesman_id;

-- HAVING pamt b/w 2000 and 6000 (both inclusive):
-- HAVING pamt BETWEEN 2000 AND 6000;
--
-- Filtering rows for pamt if its either (2000, 3000, 5760, 6000)
-- HAVING pamt IN (2000, 3000, 5760, 6000);
--
-- WHERE customer_id BETWEEN 3002 AND 3007;
SELECT
  customer_id,
  ord_date,
  MAX(purch_amt) as pamt
FROM
  orders
GROUP BY
  customer_id,
  ord_date
HAVING
  pamt > 2000;

SELECT
  customer_id,
  MAX(purch_amt)
FROM
  orders
WHERE
  customer_id BETWEEN 3002 AND 3007
GROUP BY
  customer_id
HAVING
  MAX(purch_amt) > 1000;

SELECT
  salesman_id,
  MAX(purch_amt)
FROM
  orders
WHERE
  salesman_id BETWEEN 5003 AND 5008
GROUP BY
  salseman_id;

SELECT
  COUNT(*)
FROM
  orders
WHERE
  ord_date = '2012-08-17';

SELECT
  ord_date,
  salesman_id,
  COUNT(ord_no)
FROM
  orders
GROUP BY
  ord_date,
  salesman_id;

SELECT
  COUNT(*) AS "Number of Products"
FROM
  item_mast
WHERE
  PRO_PRICE >= 350;

SELECT
  pro_com as "Company ID",
  AVG(pro_price) AS "Average Price"
FROM
  item_mast
GROUP BY
  pro_com;

SELECT
  emp_dept,
  COUNT(emp_idno)
FROM
  emp_details
GROUP BY
  emp_dept;
