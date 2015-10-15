//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef DATABLOCK_H
#define DATABLOCK_H

#include <QString>
#include <QVariant>
#include <QHash>

#define GP_VERSION 0x010000

namespace GrumpyProtocol
{
    //! List of all data types that we support used for serialization and deserialization
    //! this exists so that we can support different Qt versions for client and server
    //! otherwise we could use enum that is in Qt headers
    enum DataType
    {
        DataType_ByteArray,
        DataType_Bitmap,
        DataType_Bool,
        DataType_Int,
        DataType_UInt,
        DataType_Long,
        DataType_ULong,
        DataType_String,
        DataType_List,
        DataType_Hash,
        DataType_StringStringHash,
        DataType_StringVariantHash,
        DataType_Variant
    };

    class DataBlock
    {
        public:
            DataBlock();
            //! Return a size in bytes of header
            unsigned long GetHSize();
            unsigned long long GetSize();
            QByteArray GetHeader();
            QByteArray ToArray();
            void InsertData(QString key, QVariant value);
        private:
            QHash<QString, QVariant> data;

        //signals:


    };
}

#endif // DATABLOCK_H
