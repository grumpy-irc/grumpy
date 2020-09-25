-- This file updates the schema to version 3

/* Personal storage for users */
CREATE TABLE user_data
(
    "id" INTEGER PRIMARY KEY NOT NULL,
    "user_id" INTEGER NOT NULL,
    "key" TEXT NOT NULL,
    "data" BLOB
);

UPDATE meta SET value = '3' WHERE key = 'version';

