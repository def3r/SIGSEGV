CREATE TABLE Client_Master (
  ClientNo VARCHAR(20) PRIMARY KEY CHECK ( ClientNo LIKE 'C%' ),
  Name     VARCHAR(20) NOT NULL,
  Address1 VARCHAR(20),
  Address2 VARCHAR(20),
  City     VARCHAR(20),
  Pincode  UINT32      CHECK (Pincode >= 100000 AND Pincode <=999999),
  State    VARCHAR(10),
  BalDue   UINT32
);

CREATE TABLE Product_Master (
  ProductNo     VARCHAR(20) PRIMARY KEY CHECK (ProductNo LIKE 'P%'),
  Description   VARCHAR(40) NOT NULL,
  ProfitPercent UINT8       NOT NULL CHECK ( ProfitPercent <= 100 ),
  UnitMeasure   VARCHAR(20) NOT NULL,
  QtyOnHand     UINT32      NOT NULL,
  ReorderVI     UINT32      NOT NULL,
  SellPrice     UINT32      NOT NULL CHECK ( SellPrice != 0 ),
  CostPrice     UINT32      NOT NULL CHECK ( CostPrice != 0 )
);

CREATE TABLE SalesMan_Master (
  SalesmanNo VARCHAR(20) PRIMARY KEY CHECK ( SalesmanNo LIKE 'S%' ),
  Name       VARCHAR(40) NOT NULL,
  Address1   VARCHAR(20) NOT NULL,
  Address2   VARCHAR(20),
  City       VARCHAR(20),
  Pincode    UINT32      CHECK (Pincode >= 100000 AND Pincode <=999999),
  State      VARCHAR(10),
  TgToGet    UINT32      NOT NULL CHECK ( TgToGet != 0 ),
  Remarks    VARCHAR(20)
);

CREATE TABLE Sales_order (
  OrderNo     VARCHAR(20) PRIMARY KEY,
  ClientNo    VARCHAR(20) REFERENCES Client_Master(ClientNo),
  Orderdate   DATE,
  SalesmanNo  VARCHAR(20) REFERENCES SalesMan_Master(SalesmanNo),
  DelayType   CHAR(1)     DEFAULT 'F/P',
  DelayDate   DATE,
  OrderStatus ENUM('PROCESS', 'FULLFILLED', 'CANCELLED')
);

CREATE TABLE Sales_order_details (
  OrderNo     VARCHAR(20) REFERENCES Sales_order(OrderNo),
  ProductNo   VARCHAR(20) REFERENCES Product_Master(ProductNo),
  QtyOrdered  UINT32,
  QtyDispatch UINT32,
  ProductRate UINT32
);

INSERT INTO Client_Master VALUES
('C100', 'John Doe', 'Bhopla Street 2','Bhopal Layout','Bhopal', 224415, 'MP', 150),
('C200', 'Jane Do', 'Paris Street 2','Paris Layout','Paris', 224415, 'MH', 150),
('C300', 'Johnny Doe', 'New York Street 2','New York Layout','New York', 224415, 'UP', 150),
('C400', 'Jon De Mua', 'London Street 2','London Layout','London', 224415, 'UK', 150),
('C500', 'Juan Dee', 'Maine Street 2','Maine Layout','Maine', 224415, 'TN', 150),
('C600', 'Joe Biden', 'Tel Aviv Street 2','Tel Aviv Layout','Tel Aviv', 224415, 'TK', 150);

INSERT INTO Product_Master VALUES
('P66', '2TB Nvme SSD', 20, 'KM', 40, 10, 9000, 4000),
('P67', '1TB Nvme SSD', 30, 'KM', 30, 20, 5000, 2000),
('P68', '512GB Nvme SSD', 30, 'KM', 20, 30, 2500, 1000),
('P69', '256GB Nvme SSD', 40, 'KM', 10, 40, 2000, 1500),
('P70', '128GB Nvme SSD', 50, 'KM', 50, 50, 1000, 400),
