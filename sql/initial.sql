/*PikaCRM database version 1*/

CREATE TABLE "BuyItem" ("o_id" INTEGER NOT NULL , "b_id" INTEGER PRIMARY KEY NOT NULL , "m_id" INTEGER NOT NULL, "m_model" CHAR, "m_name" CHAR, "m_price" DOUBLE, "b_price" FLOAT NOT NULL , "b_number" INTEGER NOT NULL);

CREATE TABLE "Contact" ("co_id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "c_id" INTEGER, "co_name" CHAR NOT NULL , "co_phone" CHAR, "co_address" CHAR, "co_email" CHAR, "co_ctime" DATETIME NOT NULL  DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime')), "co_profile" BLOB, "co_card" BLOB, "co_0" BLOB, "co_1" BLOB, "co_2" BLOB, "co_3" BLOB, "co_4" BLOB, "co_5" BLOB, "co_6" BLOB, "co_7" BLOB, "co_8" BLOB, "co_9" BLOB);

CREATE TABLE "Customer" ("c_id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "c_title" CHAR NOT NULL , "c_phone" CHAR, "c_address" CHAR, "c_email" CHAR, "c_website" CHAR, "c_ccount" INTEGER NOT NULL  DEFAULT 0, "c_ctime" DATETIME NOT NULL  DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime')), "c_0" BLOB, "c_1" BLOB, "c_2" BLOB, "c_3" BLOB, "c_4" BLOB, "c_5" BLOB, "c_6" BLOB, "c_7" BLOB, "c_8" BLOB, "c_9" BLOB);

CREATE TABLE "Event" ("e_id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "c_id" INTEGER NOT NULL , "e_ask" TEXT NOT NULL , "e_status" CHAR, "e_rtime" DATETIME, "e_ctime" DATETIME NOT NULL  DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime')), "e_note" TEXT);

CREATE TABLE "Field" ("f_table" CHAR NOT NULL , "f_rowid" INTEGER NOT NULL , "f_name" CHAR NOT NULL , PRIMARY KEY ("f_table", "f_rowid"));

CREATE TABLE "LinkContact" ("o_id" INTEGER NOT NULL , "co_id" INTEGER NOT NULL , PRIMARY KEY ("o_id", "co_id"));

CREATE TABLE "Merchandise" ("m_id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL ,"m_model" CHAR , "m_name" CHAR NOT NULL , "m_price" DOUBLE, "m_0" BLOB, "m_1" BLOB, "m_2" BLOB, "m_3" BLOB, "m_4" BLOB, "m_5" BLOB, "m_6" BLOB, "m_7" BLOB, "m_8" BLOB, "m_9" BLOB);

CREATE TABLE "Orders" ("o_id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "c_id" INTEGER NOT NULL , "o_ship_add" CHAR, "o_bill_add" CHAR, "o_order_date" DATETIME, "o_ship_date" DATETIME, "o_status" CHAR, "o_ctime" DATETIME NOT NULL  DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime')), "o_mtime" DATETIME, "o_note" TEXT);

CREATE TABLE "System" ("user" CHAR NOT NULL , "ctime" DATETIME NOT NULL  DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime')), "ap_ver" CHAR NOT NULL , "sqlite_ver" CHAR NOT NULL , "db_ver" INTEGER NOT NULL );

CREATE TABLE "UnlimitedField" ("u_id" INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL , "u_name" CHAR NOT NULL );

CREATE TABLE "UserCustomize" ("u_id" INTEGER NOT NULL , "c_id" INTEGER NOT NULL , "us_content" BLOB NOT NULL , PRIMARY KEY ("u_id", "c_id"));
