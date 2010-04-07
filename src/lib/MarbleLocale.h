//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//

#ifndef MARBLE_MARBLELOCALE_H
#define MARBLE_MARBLELOCALE_H

#include "marble_export.h"
#include "global.h"


namespace Marble
{

class MarbleLocalePrivate;

/**
 * @short A class that contains all localization stuff for Marble.
 *
 * The class stores properties like the Distance Unit.
 */

class MARBLE_EXPORT MarbleLocale
{
 public:
    MarbleLocale();
    ~MarbleLocale();

    void setDistanceUnit( DistanceUnit distanceUnit );
    DistanceUnit distanceUnit() const;

    void setMeasureSystem( MeasureSystem measureSystem );
    MeasureSystem measureSystem() const;

    static QString languageCode();

 private:
    Q_DISABLE_COPY( MarbleLocale )
    MarbleLocalePrivate  * const d;
};

}

#endif
