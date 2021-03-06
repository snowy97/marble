//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Konstantin Oblaukhov <oblaukhov.konstantin@gmail.com>
//

#include "OsmBoundTagHandler.h"

#include "GeoParser.h"
#include "OsmNodeFactory.h"
#include "GeoDataDocument.h"
#include "GeoDataPlacemark.h"
#include "GeoDataParser.h"
#include "GeoDataLineString.h"
#include "MarbleDebug.h"
#include "OsmElementDictionary.h"

namespace Marble
{

namespace osm
{

static GeoTagHandlerRegistrar osmBoundTagHandler( GeoParser::QualifiedName( osmTag_bound, "" ),
        new OsmBoundTagHandler() );

GeoNode* OsmBoundTagHandler::parse( GeoParser& parser ) const
{
    Q_ASSERT( parser.isStartElement() );

    GeoDataDocument* doc = geoDataDoc( parser );
    GeoDataLinearRing r;
    GeoDataPolygon *p = new GeoDataPolygon();
    QStringList l = parser.attribute( "box" ).split( ',' );
    qreal minlat = l[0].toFloat();
    qreal minlon = l[1].toFloat();
    qreal maxlat = l[2].toFloat();
    qreal maxlon = l[3].toFloat();
    r.append( GeoDataCoordinates( minlon, minlat, 0, GeoDataCoordinates::Degree ) );
    r.append( GeoDataCoordinates( maxlon, minlat, 0, GeoDataCoordinates::Degree ) );
    r.append( GeoDataCoordinates( maxlon, maxlat, 0, GeoDataCoordinates::Degree ) );
    r.append( GeoDataCoordinates( minlon, maxlat, 0, GeoDataCoordinates::Degree ) );
    p->setOuterBoundary( r );

    GeoDataPlacemark *pl = new GeoDataPlacemark();
    pl->setGeometry( p );
    pl->setVisualCategory( GeoDataFeature::None );
    pl->setStyle( &doc->style( "background" ) );
    pl->setVisible( true );
    doc->append( pl );

    mDebug() << "[OSM] Bounds: " << minlat << " " << minlon << " " << maxlat << " " << maxlon;
    return 0;
}

}

}
