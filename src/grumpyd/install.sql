-- This file installs the default database backend for grumpy

CREATE TABLE users
(
    "id" INTEGER PRIMARY KEY NOT NULL,
    "name" TEXT NOT NULL,
    "password" TEXT,
    "last_login" NUMERIC,
    "role" TEXT,
    "is_locked" NUMERIC NOT NULL
);

CREATE INDEX idx_user_name ON users(name);

CREATE TABLE roles
(
    "id" INTEGER PRIMARY KEY NOT NULL,
    "name" TEXT NOT NULL
);

CREATE TABLE privileges
(
    "id" INTEGER PRIMARY KEY NOT NULL,
    "name" TEXT NOT NULL
);

CREATE INDEX idx_privilege_name ON privileges(name);

CREATE TABLE scrollbacks
(
    "id" INTEGER PRIMARY KEY NOT NULL,
    "user_id" INTEGER
);

CREATE INDEX idx_scrollback_user_id ON scrollbacks(user_id);

CREATE TABLE scrollback_items
(
    "id" INTEGER PRIMARY KEY NOT NULL,
    "user_id" INTEGER NOT NULL,
    "scrollback_id" INTEGER NOT NULL,
    "date" NUMERIC NOT NULL,
    "type" NUMERIC NOT NULL,
    "nick" TEXT,
    "ident" TEXT,
    "host" TEXT,
    "text" TEXT
);

CREATE INDEX idx_scrollback_item_user ON scrollback_items(user_id);

CREATE TABLE settings
(
    "id" INTEGER PRIMARY KEY NOT NULL,
    "user_id" INTEGER NOT NULL,
    "name" TEXT NOT NULL,
    "value" BLOB
);

CREATE INDEX idx_settings_user_id ON settings(user_id);