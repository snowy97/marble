//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Dennis Nienhüser <earthwings@gentoo.org>
//

#include "MarbleDeclarativeWidget.h"

#include <QtGui/QPainter>

#include "MapTheme.h"
#include "Coordinate.h"
#include "Tracking.h"
#include "ZoomButtonInterceptor.h"

#include "GeoDataCoordinates.h"
#include "GeoPainter.h"
#include "MarbleWidgetInputHandler.h"
#include "MarbleMath.h"
#include "MapThemeManager.h"
#include "AbstractFloatItem.h"
#include "RenderPlugin.h"
#include "MarbleMap.h"
#include "ViewParams.h"
#include "ViewportParams.h"
#include "GeoSceneDocument.h"
#include "GeoSceneHead.h"
#include "GeoSceneZoom.h"

#include <QtGui/QSortFilterProxyModel>

namespace Marble
{
namespace Declarative
{

class StreetMapThemeModel : public QSortFilterProxyModel
{
public:
    StreetMapThemeModel( MapThemeManager* themeManager );

protected:
    virtual bool filterAcceptsRow(int sourceRow,
             const QModelIndex &sourceParent) const;

private:
    QList<QString> m_streetMapThemeIds;
};

StreetMapThemeModel::StreetMapThemeModel( MapThemeManager *themeManager )
{
    /** @todo Extend .dgml spec by categories to simplify this
      * The map theme model items should specify the planet and a set of
      * categories/tags (arbitrary strings) to simplify filtering for specific
      * map theme properties.
      * E.g. the item for earth/openstreetmap/openstreetmap.dgml should have
      * the planet set to earth and categories/tags like "OpenStreetMap, street map"
      */

    QStringList const themes = themeManager->mapThemeIds();
    foreach( const QString &theme, themes ) {
        if ( theme.startsWith( "earth/" ) ) {
            GeoSceneDocument* document = themeManager->loadMapTheme( theme );
            if ( document && document->head()->zoom()->maximum() > 3000 ) {
                m_streetMapThemeIds << document->head()->mapThemeId();
                delete document;
            }
        }
    }
}

bool StreetMapThemeModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
     QModelIndex const index = sourceModel()->index( sourceRow, 0, sourceParent );
     return m_streetMapThemeIds.contains( index.data( Qt::UserRole + 1 ).toString() );
}

MarbleWidget::MarbleWidget( QDeclarativeItem *parent ) :
    QDeclarativeItem( parent ),
    m_model(),
    m_map( &m_model ),
    m_inputEnabled( true ),
    m_tracking( 0 ),
    m_routing( 0 ),
    m_search( 0 ),
    m_interceptor( new ZoomButtonInterceptor( this, this ) ),
    m_streetMapThemeModel( 0 )
{
    m_map.setMapThemeId( "earth/openstreetmap/openstreetmap.dgml" );
    setFlag( QGraphicsItem::ItemHasNoContents, false );  // enable painting

    connect( &m_map, SIGNAL( visibleLatLonAltBoxChanged( GeoDataLatLonAltBox ) ),
             this, SIGNAL( visibleLatLonAltBoxChanged( ) ) );
    connect( &m_map, SIGNAL( visibleLatLonAltBoxChanged( GeoDataLatLonAltBox ) ),
             this, SIGNAL( zoomChanged() ) );
    connect( &m_map, SIGNAL( themeChanged( const QString & ) ),
             this, SIGNAL( mapThemeChanged() ) );
    connect( &m_map, SIGNAL( mouseClickGeoPosition( qreal, qreal, GeoDataCoordinates::Unit ) ),
             this, SLOT( forwardMouseClick( qreal, qreal, GeoDataCoordinates::Unit ) ) );

    connect( &m_model, SIGNAL( workOfflineChanged() ),
             this, SIGNAL( workOfflineChanged() ) );

    connect( &m_center, SIGNAL(latitudeChanged()), this, SLOT(updateCenterPosition()));
    connect( &m_center, SIGNAL(longitudeChanged()), this, SLOT(updateCenterPosition()));

// FIXME    m_marbleWidget->inputHandler()->setMouseButtonPopupEnabled( Qt::LeftButton, false );
// FIXME    m_marbleWidget->inputHandler()->setPanViaArrowsEnabled( false );
}

void MarbleWidget::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    bool  doClip = true;
    if ( m_map.projection() == Spherical )
        doClip = ( m_map.radius() > width() / 2
                   || m_map.radius() > height() / 2 );

    if ( m_map.size() != QSize( width(), height() ) ) {
        m_map.setSize( QSize( width(), height() ) );
    }

    QPixmap px( width(), height() );
    if ( !m_map.mapCoversViewport() ) {
        px.fill( Qt::black );
    }

    // Create a painter that will do the painting.
    GeoPainter geoPainter( &px, m_map.viewport(), m_map.mapQuality(), doClip );

    m_map.setViewContext( smooth() ? Still : Animation );
    m_map.paint( geoPainter, QRect() );

    painter->drawPixmap( 0, 0, width(), height(), px );
}


Marble::MarbleModel *MarbleWidget::model()
{
    return &m_model;
}

const Marble::ViewportParams *MarbleWidget::viewport() const
{
    return m_map.viewport();
}

QList<RenderPlugin *> MarbleWidget::renderPlugins() const
{
    return m_map.renderPlugins();
}

QStringList MarbleWidget::activeFloatItems() const
{
    QStringList result;
    foreach( AbstractFloatItem * floatItem, m_map.floatItems() ) {
        if ( floatItem->enabled() && floatItem->visible() ) {
            result << floatItem->nameId();
        }
    }
    return result;
}

void MarbleWidget::setActiveFloatItems( const QStringList &items )
{
    foreach( AbstractFloatItem * floatItem, m_map.floatItems() ) {
        floatItem->setEnabled( items.contains( floatItem->nameId() ) );
        floatItem->setVisible( items.contains( floatItem->nameId() ) );
    }
}

QStringList MarbleWidget::activeRenderPlugins() const
{
    QStringList result;
    foreach( RenderPlugin * plugin, m_map.renderPlugins() ) {
        if ( plugin->enabled() && plugin->visible() ) {
            result << plugin->nameId();
        }
    }
    return result;
}

void MarbleWidget::setActiveRenderPlugins( const QStringList &items )
{
    foreach( RenderPlugin * plugin, m_map.renderPlugins() ) {
        plugin->setEnabled( items.contains( plugin->nameId() ) );
        plugin->setVisible( items.contains( plugin->nameId() ) );
    }
}

bool MarbleWidget::inputEnabled() const
{
    return m_inputEnabled;
}

void MarbleWidget::setInputEnabled( bool enabled )
{
    m_inputEnabled = enabled;
// FIXME    m_marbleWidget->setInputEnabled( enabled );
}

QString MarbleWidget::mapThemeId() const
{
    return m_map.mapThemeId();
}

void MarbleWidget::setMapThemeId( const QString &mapThemeId )
{
    m_map.setMapThemeId( mapThemeId );
}

QString MarbleWidget::projection( ) const
{
    switch ( m_map.projection() ) {
    case Equirectangular:
        return "Equirectangular";
    case Mercator:
        return "Mercator";
    case Spherical:
        return "Spherical";
    }

    Q_ASSERT( false && "Marble got a new projection which we do not know about yet" );
    return "Spherical";
}

void MarbleWidget::setProjection( const QString &projection )
{
    if ( projection.compare( "Equirectangular", Qt::CaseInsensitive ) == 0 ) {
        m_map.setProjection( Equirectangular );
    } else if ( projection.compare( "Mercator", Qt::CaseInsensitive ) == 0 ) {
        m_map.setProjection( Mercator );
    } else {
        m_map.setProjection( Spherical );
    }
}

void MarbleWidget::zoomIn()
{
    const int zoomStep = 60;

    setZoom( zoom() + zoomStep );
}

void MarbleWidget::zoomOut()
{
    const int zoomStep = 60;

    setZoom( zoom() - zoomStep );
}

QPoint MarbleWidget::pixel( qreal lon, qreal lat ) const
{
    GeoDataCoordinates position( lon, lat, 0, GeoDataCoordinates::Degree );
    qreal x( 0.0 );
    qreal y( 0.0 );
    const ViewportParams *viewport = m_map.viewport();
    viewport->screenCoordinates( position, x, y );
    return QPoint( x, y );
}

Coordinate *MarbleWidget::coordinate( int x, int y )
{
    qreal lat( 0.0 ), lon( 0.0 );
    m_map.geoCoordinates( x, y, lon, lat );
    return new Coordinate( lon, lat, 0.0, this );
}

Marble::Declarative::Tracking* MarbleWidget::tracking()
{
    if ( !m_tracking ) {
        m_tracking = new Tracking( this );
        m_tracking->setMarbleWidget( this );
        emit trackingChanged();
    }

    return m_tracking;
}

Coordinate* MarbleWidget::center()
{
    m_center.blockSignals( true );
    m_center.setLongitude( m_map.centerLongitude() );
    m_center.setLatitude( m_map.centerLatitude() );
    m_center.blockSignals( false );
    return &m_center;
}

void MarbleWidget::setCenter( Coordinate* center )
{
    if ( center ) {
        m_center.blockSignals( true );
        m_center.setLongitude( center->longitude() );
        m_center.setLatitude( center->latitude() );
        m_center.setAltitude( center->altitude() );
        m_center.blockSignals( false );
        updateCenterPosition();
    }
}

void MarbleWidget::centerOn( const Marble::GeoDataLatLonAltBox &bbox )
{
    //prevent divide by zero
    if( bbox.height() && bbox.width() ) {
        //work out the needed zoom level
        const int horizontalRadius = ( 0.25 * M_PI ) * ( m_map.height() / bbox.height() );
        const int verticalRadius = ( 0.25 * M_PI ) * ( m_map.width() / bbox.width() );
        const int newRadius = qMin<int>( horizontalRadius, verticalRadius );
        m_map.setRadius( newRadius );
    }

    m_map.centerOn( bbox.center().longitude( GeoDataCoordinates::Degree ), bbox.center().latitude( GeoDataCoordinates::Degree ) );
}

void MarbleWidget::updateCenterPosition()
{
    m_map.centerOn( m_center.longitude(), m_center.latitude() );
    update();
}

void MarbleWidget::forwardMouseClick(qreal lon, qreal lat, GeoDataCoordinates::Unit unit )
{
    GeoDataCoordinates position( lon, lat, unit );
    emit mouseClickGeoPosition( position.longitude( GeoDataCoordinates::Degree ), position.latitude( GeoDataCoordinates::Degree ) );
}

Marble::Declarative::Routing* MarbleWidget::routing()
{
    if ( !m_routing ) {
        m_routing = new Routing( this );
        m_routing->setMarbleWidget( this );
    }

    return m_routing;
}

Marble::Declarative::Search* MarbleWidget::search()
{
    if ( !m_search ) {
        m_search = new Search( this );
        m_search->setMarbleWidget( this );
        m_search->setDelegateParent( this );
    }

    return m_search;
}

QObject *MarbleWidget::mapThemeModel()
{
    return m_map.model()->mapThemeManager()->mapThemeModel();
}

QObject* MarbleWidget::streetMapThemeModel()
{
    if ( !m_streetMapThemeModel ) {
        StreetMapThemeModel* model = new StreetMapThemeModel( m_model.mapThemeManager() );
        model->setSourceModel( m_model.mapThemeManager()->mapThemeModel() );
        m_streetMapThemeModel = model;
        return m_streetMapThemeModel;
    }

    return 0;
}

void MarbleWidget::setGeoSceneProperty(const QString &key, bool value)
{
    m_map.setPropertyValue( key, value );
}

bool MarbleWidget::workOffline() const
{
    return m_model.workOffline();
}

void MarbleWidget::setWorkOffline( bool workOffline )
{
    m_model.setWorkOffline( workOffline );
}

int MarbleWidget::zoom() const
{
    return (200.0 * qLn( m_map.radius() ) );
}

void MarbleWidget::setZoom( int zoom )
{
    const qreal radius = qPow( M_E, ( zoom / 200.0 ) );

    m_map.setRadius( radius );
    update();
}

}
}

#include "MarbleDeclarativeWidget.moc"
