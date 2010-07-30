//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Dennis Nienhüser <earthwings@gentoo.org>
//


#ifndef MARBLE_NOMINATIMPLUGIN_H
#define MARBLE_NOMINATIMPLUGIN_H

#include "RunnerPlugin.h"

namespace Marble
{

class NominatimPlugin : public RunnerPlugin
{
    Q_OBJECT
public:
    explicit NominatimPlugin( QObject *parent = 0 );

    virtual MarbleAbstractRunner* newRunner() const;
};

}

#endif