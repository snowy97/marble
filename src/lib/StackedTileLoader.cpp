/**
 * This file is part of the Marble Desktop Globe.
 *
 * Copyright 2005-2007 Torsten Rahn <tackat@kde.org>
 * Copyright 2007      Inge Wallin  <ingwa@kde.org>
 * Copyright 2008, 2009, 2010 Jens-Michael Hoffmann <jensmh@gmx.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "StackedTileLoader.h"

#include "TextureTile.h"
#include "TileLoader.h"
#include "MarbleModel.h"
#include "global.h"
#include "GeoSceneDocument.h"
#include "GeoSceneHead.h"
#include "GeoSceneLayer.h"
#include "GeoSceneMap.h"
#include "GeoSceneTexture.h"
#include "HttpDownloadManager.h"
#include "DatasetProvider.h"
#include "StackedTile.h"
#include "MapThemeManager.h"
#include "MarbleDirs.h"
#include "TileLoaderHelper.h"
#include "MarbleDebug.h"

#include <QtCore/QCache>
#include <QtCore/QDateTime>
#include <QtCore/QHash>
#include <QtGui/QImage>


#ifdef Q_CC_MSVC
# ifndef KDEWIN_MATH_H
   long double log(int i) { return log((long double)i); }
# endif
#endif

namespace Marble
{

class StackedTileLoaderPrivate
{
public:
    StackedTileLoaderPrivate()
        : m_datasetProvider( 0 ),
          m_mapThemeManager( 0 ),
          m_simpleTileLoader( 0 )
    {
        m_tileCache.setMaxCost( 20000 * 1024 ); // Cache size measured in bytes
    }

    DatasetProvider *m_datasetProvider;
    MapThemeManager const *m_mapThemeManager;
    // TODO: comment about uint hash key
    QHash<uint, GeoSceneLayer const *> m_sceneLayers;
    QHash<uint, GeoSceneTexture*> m_textureLayers;
    TileLoader *m_simpleTileLoader;
    QHash <TileId, StackedTile*>  m_tilesOnDisplay;
    QCache <TileId, StackedTile>  m_tileCache;
};


StackedTileLoader::StackedTileLoader( MapThemeManager const * const mapThemeManager,
                                      HttpDownloadManager * const downloadManager,
                                      MarbleModel * const model )
    : d( new StackedTileLoaderPrivate ),
      m_parent( model )
{
    d->m_mapThemeManager = mapThemeManager;
    initTextureLayers();
    d->m_simpleTileLoader = new TileLoader( mapThemeManager, downloadManager );
    d->m_simpleTileLoader->setTextureLayers( d->m_textureLayers );
    connect( d->m_simpleTileLoader, SIGNAL( tileCompleted( TileId, TileId )),
             SLOT( updateTile( TileId, TileId )));
    setDownloadManager( downloadManager );
}

StackedTileLoader::~StackedTileLoader()
{
    flush();
    d->m_tileCache.clear();
    delete d->m_simpleTileLoader;
    delete d;
}

void StackedTileLoader::setDownloadManager( HttpDownloadManager *downloadManager )
{
}

void StackedTileLoader::resetTilehash()
{
    QHash<TileId, StackedTile*>::const_iterator it = d->m_tilesOnDisplay.constBegin();
    QHash<TileId, StackedTile*>::const_iterator const end = d->m_tilesOnDisplay.constEnd();
    for (; it != end; ++it ) {
        it.value()->setUsed( false );
    }
}

void StackedTileLoader::cleanupTilehash()
{
    // Make sure that tiles which haven't been used during the last
    // rendering of the map at all get removed from the tile hash.

    QHashIterator<TileId, StackedTile*> it( d->m_tilesOnDisplay );
    while ( it.hasNext() ) {
        it.next();
        if ( !it.value()->used() ) {
            // If insert call result is false then the cache is too small to store the tile
            // but the item will get deleted nevertheless and the pointer we have
            // doesn't get set to zero (so don't delete it in this case or it will crash!)
            d->m_tileCache.insert( it.key(), it.value(), it.value()->numBytes() );
            d->m_tilesOnDisplay.remove( it.key() );
        }
    }
}

void StackedTileLoader::flush()
{
    // Remove all tiles from m_tilesOnDisplay
    QHashIterator<TileId, StackedTile*> it( d->m_tilesOnDisplay );
    while ( it.hasNext() ) {
        it.next();
        // If insert call result is false then the cache is too small to store the tile
        // but the item will get deleted nevertheless and the pointer we have
        // doesn't get set to zero (so don't delete it in this case or it will crash!)
        d->m_tileCache.insert( it.key(), it.value(), it.value()->numBytes() );
        d->m_tilesOnDisplay.remove( it.key() );
    }

    d->m_tilesOnDisplay.clear();
}

StackedTile* StackedTileLoader::loadTile( TileId const &tileId, bool const forMergedLayerDecorator )
{
    // check if the tile is in the hash
    StackedTile * tile = d->m_tilesOnDisplay.value( tileId, 0 );
    if ( tile ) {
        tile->setUsed( true );
        return tile;
    }
    // here ends the performance critical section of this method

    mDebug() << "StackedTileLoader::loadTile" << tileId.toString();

    // the tile was not in the hash or has been removed because of expiration
    // so check if it is in the cache
    tile = d->m_tileCache.take( tileId );
    if ( tile ) {
        // the tile was in the cache, but is it up to date?
        if ( !tile->expired() ) {
            d->m_tilesOnDisplay[ tileId ] = tile;
            tile->setUsed( true );
            return tile;
        } else {
            delete tile;
            tile = 0;
        }
    }

    GeoSceneTexture * const texture = d->m_textureLayers[ tileId.mapThemeIdHash() ];

    // tile (valid) has not been found in hash or cache, so load it from disk
    // and place it in the hash from where it will get transferred to the cache

    // mDebug() << "load Tile from Disk: " << tileId.toString();
    tile = new StackedTile( tileId );
    if ( forMergedLayerDecorator )
        tile->setForMergedLayerDecorator();
    d->m_tilesOnDisplay[ tileId ] = tile;

    GeoSceneLayer const * const sceneLayer = d->m_sceneLayers.value( tileId.mapThemeIdHash(), 0 );
    Q_ASSERT( sceneLayer );
    mDebug() << "scene layer:" << sceneLayer->name();

    QVector<GeoSceneAbstractDataset*> textureLayers = sceneLayer->datasets();
    mDebug() << "StackedTileLoader::loadTile: loading base tiles:"
             << textureLayers.size() << "texture layers found";

    QVector<GeoSceneAbstractDataset*>::const_iterator pos = textureLayers.constBegin();
    QVector<GeoSceneAbstractDataset*>::const_iterator const end = textureLayers.constEnd();
    for (; pos != end; ++pos ) {
        GeoSceneTexture const * const textureLayer = dynamic_cast<GeoSceneTexture const *>( *pos );
        if ( !textureLayer )
            mDebug() << "not a texture layer" << (*pos)->name();
        if ( textureLayer && ( !textureLayer->hasMaximumTileLevel()
                               || tileId.zoomLevel() <= textureLayer->maximumTileLevel() )) {
            TileId const simpleTileId( textureLayer->sourceDir(), tileId.zoomLevel(),
                                       tileId.x(), tileId.y() );
            mDebug() << "StackedTileLoader::loadTile: base tile" << textureLayer->sourceDir() << simpleTileId.toString();
            TextureTile * const simpleTile = d->m_simpleTileLoader->loadTile( tileId, simpleTileId );
            // hack to try clouds
            if ( simpleTile && tile->hasBaseTiles() )
                simpleTile->setMergeRule( TextureTile::MergeMultiply );
            if ( simpleTile )
                tile->addBaseTile( simpleTile );
        }
    }
    Q_ASSERT( tile->hasBaseTiles() );

    if ( tile->state() != StackedTile::TileEmpty ) {
        tile->initResultTile();
        mergeDecorations( tile, texture );
    }
    return tile;
}

quint64 StackedTileLoader::volatileCacheLimit() const
{
    return d->m_tileCache.maxCost() / 1024;
}

QList<TileId> StackedTileLoader::tilesOnDisplay() const
{
    QList<TileId> result;
    QHash<TileId, StackedTile*>::const_iterator pos = d->m_tilesOnDisplay.constBegin();
    QHash<TileId, StackedTile*>::const_iterator const end = d->m_tilesOnDisplay.constEnd();
    for (; pos != end; ++pos ) {
        if ( pos.value()->used() ) {
            result.append( pos.key() );
        }
    }
    return result;
}

int StackedTileLoader::maximumTileLevel( GeoSceneTexture const * const texture )
{
    if ( !texture )
        return -1;

    // if maximum tile level is configured in the DGML files,
    // then use it, otherwise use old detection code.
    if ( texture->maximumTileLevel() >= 0 )
        return texture->maximumTileLevel();

    int maximumTileLevel = -1;
    QString tilepath = MarbleDirs::path( TileLoaderHelper::themeStr( texture ) );
    //    mDebug() << "StackedTileLoader::maxPartialTileLevel tilepath" << tilepath;
    QStringList leveldirs = QDir( tilepath ).entryList( QDir::AllDirs | QDir::NoSymLinks
                                                        | QDir::NoDotAndDotDot );

    bool ok = true;

    QStringList::const_iterator it = leveldirs.constBegin();
    QStringList::const_iterator const end = leveldirs.constEnd();
    for (; it != end; ++it ) {
        int value = (*it).toInt( &ok, 10 );
        if ( ok && value > maximumTileLevel )
            maximumTileLevel = value;
    }

    //    mDebug() << "Detected maximum tile level that contains data: "
    //             << maxtilelevel;
    return maximumTileLevel + 1;
}

bool StackedTileLoader::baseTilesAvailable( GeoSceneLayer * layer )
{
    if ( !layer ) return false;

    GeoSceneTexture * texture = static_cast<GeoSceneTexture *>( layer->groundDataset() );

    const int  levelZeroColumns = texture->levelZeroColumns();
    const int  levelZeroRows    = texture->levelZeroRows();

    bool noerr = true;

    // Check whether the tiles from the lowest texture level are available
    //
    for ( int column = 0; noerr && column < levelZeroColumns; ++column ) {
        for ( int row = 0; noerr && row < levelZeroRows; ++row ) {

            const QString tilepath = MarbleDirs::path( TileLoaderHelper::relativeTileFileName(
                texture, 0, column, row ));
            noerr = QFile::exists( tilepath );
        }
    }

    return noerr;
}

void StackedTileLoader::setVolatileCacheLimit( quint64 kiloBytes )
{
    mDebug() << QString("Setting tile cache to %1 kilobytes.").arg( kiloBytes );
    d->m_tileCache.setMaxCost( kiloBytes * 1024 );
}

void StackedTileLoader::updateTile( TileId const & composedTileId, TileId const & baseTileId )
{
    StackedTile * const tile = d->m_tilesOnDisplay.value( composedTileId, 0 );
    if ( tile ) {
        tile->deriveCompletionState();
        tile->initResultTile();
        mergeDecorations( tile, findTextureLayer( composedTileId ));
        emit tileUpdateAvailable();
    } else
        // TODO: also update tiles in the cache, not doing it is really a waste of i/o
        d->m_tileCache.remove( composedTileId );
}

void StackedTileLoader::update()
{
    mDebug() << "StackedTileLoader::update()";
    flush(); // trigger a reload of all tiles that are currently in use
    d->m_tileCache.clear(); // clear the tile cache in physical memory
    emit tileUpdateAvailable();
}

inline GeoSceneTexture const * StackedTileLoader::findTextureLayer( TileId const & id ) const
{
    GeoSceneTexture const * const textureLayer = d->m_textureLayers.value( id.mapThemeIdHash(), 0 );
    Q_ASSERT( textureLayer );
    return textureLayer;
}

inline GeoSceneTexture * StackedTileLoader::findTextureLayer( TileId const & id )
{
    GeoSceneTexture * const textureLayer = d->m_textureLayers.value( id.mapThemeIdHash(), 0 );
    Q_ASSERT( textureLayer );
    return textureLayer;
}

void StackedTileLoader::initTextureLayers()
{
    QList<GeoSceneDocument const *> const & mapThemes = d->m_mapThemeManager->mapThemes();
    QList<GeoSceneDocument const *>::const_iterator pos = mapThemes.constBegin();
    QList<GeoSceneDocument const *>::const_iterator const end = mapThemes.constEnd();
    for (; pos != end; ++pos ) {
        GeoSceneHead const * head = (*pos)->head();
        Q_ASSERT( head );
        const QString mapThemeId = head->target() + '/' + head->theme();
        mDebug() << "StackedTileLoader::initTextureLayers" << mapThemeId;

        GeoSceneMap const * map = (*pos)->map();
        Q_ASSERT( map );
        GeoSceneLayer const * sceneLayer = map->layer( head->theme() );
        if ( !sceneLayer ) {
            mDebug() << "ignoring, has no GeoSceneLayer for" << head->theme();
            continue;
        }

        d->m_sceneLayers.insert( qHash( mapThemeId ), sceneLayer );

        // find all texture layers
        QVector<GeoSceneAbstractDataset *> layers = sceneLayer->datasets();
        QVector<GeoSceneAbstractDataset *>::const_iterator pos = layers.constBegin();
        QVector<GeoSceneAbstractDataset *>::const_iterator const end = layers.constEnd();
        for (; pos != end; ++pos ) {
            GeoSceneTexture * const textureLayer = dynamic_cast<GeoSceneTexture *>( *pos );
            if ( !textureLayer ) {
                mDebug() << "ignoring dataset, is not a texture layer";
                continue;
            }
            d->m_textureLayers.insert( qHash( textureLayer->sourceDir() ), textureLayer );
            mDebug() << "StackedTileLoader::initTextureLayers" << "added texture layer:"
                     << qHash( textureLayer->sourceDir() ) << textureLayer->sourceDir();
        }
    }
}

void StackedTileLoader::mergeDecorations( StackedTile * const tile,
                                   GeoSceneTexture * const textureLayer ) const
{
    Q_ASSERT( tile->state() != StackedTile::TileEmpty );
    if ( !tile->forMergedLayerDecorator() )
        m_parent->paintTile( tile, textureLayer );
}

}

#include "StackedTileLoader.moc"