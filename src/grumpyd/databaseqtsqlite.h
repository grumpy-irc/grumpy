//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef DATABASEQTSQLITE_H
#define DATABASEQTSQLITE_H

#include "databaseqtsql.h"

#define GRUMPYD_SCHEMA_VERSION 3

namespace GrumpyIRC
{
    class DatabaseQtSqlite : public DatabaseQtSQL
    {
        public:
            DatabaseQtSqlite();
            void Maintenance_Specific() override;
        protected:
            void init() override;
            bool install() override;
        private:
            void UpdateDB(unsigned int patch);
            QString datafile;
    };
}

#endif // DATABASEQTSQLITE_H
