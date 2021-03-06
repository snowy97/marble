//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
//

#include "GeoPointGraphicsItem.h"

#include "GeoPainter.h"

namespace Marble
{

GeoPointGraphicsItem::GeoPointGraphicsItem()
        : GeoGraphicsItem()
{
}

void GeoPointGraphicsItem::setPoint( const GeoDataPoint& point )
{
    m_point = point;
}

GeoDataPoint GeoPointGraphicsItem::point() const
{
    return m_point;
}

void GeoPointGraphicsItem::paint( GeoPainter* painter, ViewportParams* viewport,
                                  const QString& renderPos, GeoSceneLayer* layer )
{
    Q_UNUSED( renderPos );
    Q_UNUSED( layer );
    Q_UNUSED( viewport );
    painter->drawPoint( m_point );
}

}
