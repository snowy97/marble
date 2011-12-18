//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Dennis Nienhüser <earthwings@gentoo.org>
//

#include "Coordinate.h"

#include "MarbleMath.h"

namespace Marble
{
namespace Declarative
{

Coordinate::Coordinate( QObject *parent ) :
    QObject( parent ),
    m_coordinate( 0 )
{
}

Coordinate::Coordinate( qreal lon, qreal lat, qreal alt, QObject *parent ) :
    QObject( parent ),
    m_coordinate( new GeoDataCoordinates( lon, lat, alt, GeoDataCoordinates::Degree ) )
{
}

Coordinate::~Coordinate()
{
    delete m_coordinate;
}

bool Coordinate::isValid() const
{
    return m_coordinate != 0;
}

qreal Coordinate::longitude() const
{
    if ( !m_coordinate )
        return GeoDataCoordinates().longitude( GeoDataCoordinates::Degree );

    return m_coordinate->longitude( GeoDataCoordinates::Degree );
}

void Coordinate::setLongitude( qreal lon )
{
    if ( !m_coordinate ) {
        m_coordinate = new GeoDataCoordinates();
    }

    m_coordinate->setLongitude( lon, GeoDataCoordinates::Degree );
    emit longitudeChanged();
}

qreal Coordinate::latitude() const
{
    if ( !m_coordinate )
        return GeoDataCoordinates().latitude( GeoDataCoordinates::Degree );

    return m_coordinate->latitude( GeoDataCoordinates::Degree );
}

void Coordinate::setLatitude( qreal lat )
{
    if ( !m_coordinate ) {
        m_coordinate = new GeoDataCoordinates();
    }

    m_coordinate->setLatitude( lat, GeoDataCoordinates::Degree );
    emit latitudeChanged();
}

qreal Coordinate::altitude() const
{
    if ( !m_coordinate )
        return GeoDataCoordinates().altitude();

    return m_coordinate->altitude();
}

void Coordinate::setAltitude( qreal alt )
{
    if ( !m_coordinate ) {
        m_coordinate = new GeoDataCoordinates();
    }

    m_coordinate->setAltitude( alt );
    emit altitudeChanged();
}

GeoDataCoordinates Coordinate::coordinates() const
{
    if ( !m_coordinate )
        return GeoDataCoordinates();

    return *m_coordinate;
}

void Coordinate::setCoordinates( const GeoDataCoordinates &coordinates )
{
    if ( !m_coordinate ) {
        m_coordinate = new GeoDataCoordinates();
    }

    *m_coordinate = coordinates;
}

qreal Coordinate::distance( qreal longitude, qreal latitude ) const
{
    GeoDataCoordinates::Unit deg = GeoDataCoordinates::Degree;
    GeoDataCoordinates other( longitude, latitude, 0, deg );
    return EARTH_RADIUS * distanceSphere( coordinates(), other );
}

qreal Coordinate::bearing( qreal longitude, qreal latitude ) const
{
    qreal deltaLon = longitude * DEG2RAD - this->longitude();
    qreal y = sin( deltaLon ) * cos( latitude * DEG2RAD );
    qreal x = cos( this->latitude() ) * sin( latitude * DEG2RAD ) -
              sin( this->latitude() ) * cos( latitude * DEG2RAD ) * cos( deltaLon );
    return RAD2DEG * atan2( y, x );
}

bool Coordinate::operator == ( const Coordinate &other ) const
{
    if ( m_coordinate == 0 )
        return false;

    if ( other.m_coordinate == 0 )
        return false;

    return *m_coordinate == *other.m_coordinate;
}

bool Coordinate::operator != ( const Coordinate &other ) const
{
    return !operator == ( other );
}

}
}

#include "Coordinate.moc"
