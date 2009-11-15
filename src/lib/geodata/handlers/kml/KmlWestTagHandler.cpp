//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

#include "KmlWestTagHandler.h"

#include "MarbleDebug.h"

#include "KmlElementDictionary.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoDataParser.h"

namespace Marble
{
namespace kml
{
KML_DEFINE_TAG_HANDLER( west )

GeoNode* KmlwestTagHandler::parse( GeoParser& parser ) const
{
    Q_ASSERT( parser.isStartElement() && parser.isValidElement( kmlTag_west ) );

    GeoStackItem parentItem = parser.parentElement();

    if( parentItem.represents( kmlTag_LatLonAltBox ) ) {
        float west = parser.readElementText().trimmed().toFloat();

        parentItem.nodeAs<GeoDataLatLonAltBox>()->setWest( west, GeoDataCoordinates::Degree );
#ifdef DEBUG_TAGS
        mDebug() << "Parsed <" << kmlTag_west << "> containing: " << west
                 << " parent item name: " << parentItem.qualifiedName().first;
#endif // DEBUG_TAGS
    }

    return 0;
}

}
}
