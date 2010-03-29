//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Patrick Spendrin <ps_ml@gmx.de>
//

#ifndef GEODATAGEOMETRYPRIVATE_H
#define GEODATAGEOMETRYPRIVATE_H

#include <QtCore/QString>
#include <QtCore/QAtomicInt>

#include "GeoDataGeometry.h"

#include "GeoDataTypes.h"

namespace Marble
{

class GeoDataGeometryPrivate
{
  public:
    GeoDataGeometryPrivate()
        : m_extrude( false ),
          m_altitudeMode( ClampToGround ),
          ref( 0 )
    {
    }

    GeoDataGeometryPrivate( const GeoDataGeometryPrivate& other )
        : m_extrude( other.m_extrude ),
          m_altitudeMode( other.m_altitudeMode ),
          ref( 0 )
    {
    }

    virtual ~GeoDataGeometryPrivate()
    {
    }

    virtual GeoDataGeometryPrivate* copy()
    { 
        GeoDataGeometryPrivate* copy = new GeoDataGeometryPrivate;
        *copy = *this;
        return copy;
    }

    virtual QString nodeType() const
    {
        return GeoDataTypes::GeoDataGeometryType;
    }

    virtual EnumGeometryId geometryId() const
    {
        return InvalidGeometryId;
    }

    bool         m_extrude;
    AltitudeMode m_altitudeMode;
    
    QAtomicInt  ref;
};

} // namespace Marble

#endif //GeoDataGeometryPRIVATE_H
