/*PikaCRM database version 1*/

CREATE TABLE "BuyItem" ("o_id" INTEGER NOT NULL , "b_id" INTEGER NOT NULL , "m_id" INTEGER NOT NULL , "b_price" FLOAT NOT NULL , "b_number" INTEGER NOT NULL , PRIMARY KEY ("o_id", "b_id"));

CREATE TABLE "Contact" ("co_id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "c_id" INTEGER, "co_name" CHAR NOT NULL , "co_phone" CHAR, "co_address" CHAR, "co_email" CHAR, "co_ctime" DATETIME NOT NULL  DEFAULT CURRENT_TIMESTAMP, "co_profile" BLOB, "co_card" BLOB, "co_1" BLOB, "co_2" BLOB, "co_3" BLOB);

CREATE TABLE "Customer" ("c_id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "c_title" CHAR NOT NULL , "c_phone" CHAR, "c_address" CHAR, "c_email" CHAR, "c_website" CHAR, "c_ccount" INTEGER NOT NULL  DEFAULT 0, "c_ctime" DATETIME NOT NULL  DEFAULT CURRENT_TIMESTAMP, "c_1" BLOB, "c_2" BLOB, "c_3" BLOB);

CREATE TABLE "Event" ("e_id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "c_id" INTEGER NOT NULL , "e_ask" TEXT NOT NULL , "e_status" CHAR, "e_ctime" DATETIME NOT NULL  DEFAULT CURRENT_TIMESTAMP, "e_note" TEXT);

CREATE TABLE "Field" ("f_table" CHAR NOT NULL , "f_idname" CHAR NOT NULL , "f_name" CHAR NOT NULL , PRIMARY KEY ("f_table", "f_idname"));

CREATE TABLE "LinkContact" ("o_id" INTEGER NOT NULL , "co_id" INTEGER NOT NULL , PRIMARY KEY ("o_id", "co_id"));

CREATE TABLE "Merchandise" ("m_id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "m_name" CHAR NOT NULL , "m_price" INTEGER, "m_1" BLOB, "m_2" BLOB, "m_3" BLOB);

CREATE TABLE "Order" ("o_id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "c_id" INTEGER NOT NULL , "o_ship_add" CHAR, "o_bill_add" CHAR, "o_date" DATETIME, "o_ctime" DATETIME NOT NULL  DEFAULT CURRENT_TIMESTAMP, "o_mtime" DATETIME, "o_note" TEXT);

CREATE TABLE "System" ("user" CHAR NOT NULL , "ctime" DATETIME NOT NULL  DEFAULT CURRENT_TIMESTAMP, "ap_ver" CHAR NOT NULL , "sqlite_ver" CHAR NOT NULL , "db_ver" CHAR NOT NULL );

CREATE TABLE "Unlimited" ("u_id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "u_name" CHAR NOT NULL );

CREATE TABLE "UserCustomize" ("u_id" INTEGER NOT NULL , "c_id" INTEGER NOT NULL , "us_content" BLOB NOT NULL , PRIMARY KEY ("u_id", "c_id"));
