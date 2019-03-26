//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2019

#ifndef SCRIPTCOMMAND_H
#define SCRIPTCOMMAND_H

#include "../commandprocessor.h"
#include <QJSEngine>

namespace GrumpyIRC
{
    class ScriptExtension;
    class LIBCORESHARED_EXPORT ScriptCommand : public SystemCommand
    {
        public:
            ScriptCommand(const QString &name, ScriptExtension *e, const QString &function);
            ~ScriptCommand() override;
            ScriptExtension *GetScript();
            QString GetFN();
            QJSEngine *GetEngine();
            int Run(const CommandArgs &args) override;

        private:
            ScriptExtension *script;
            QString fn;
    };
}

#endif // SCRIPTCOMMAND_H
