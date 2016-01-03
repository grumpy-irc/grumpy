-- This file updates the schema to version 2

ALTER TABLE scrollbacks
    ADD COLUMN "property" BLOB;

UPDATE meta SET value = '2' WHERE key = 'version';

