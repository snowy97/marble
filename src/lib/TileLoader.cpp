// Copyright 2010 Jens-Michael Hoffmann <jmho@c-xx.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "TileLoader.h"

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtGui/QImage>

#include "GeoSceneTexture.h"
#include "HttpDownloadManager.h"
#include "MarbleDebug.h"
#include "MarbleDirs.h"
#include "TextureTile.h"
#include "TileLoaderHelper.h"

namespace Marble
{

TileLoader::TileLoader( GeoSceneTexture const * textureLayer, HttpDownloadManager * const downloadManager )
    : m_textureLayer( textureLayer )
    , m_downloadManager( downloadManager )
{
}

// If the tile is locally available:
//     - if not expired: create TextureTile, set state to "uptodate", return it => done
//     - if expired: create TextureTile, state is set to Expired by default, trigger dl,

QSharedPointer<TextureTile> TileLoader::loadTile( TileId const & stackedTileId,
                                                  TileId const & tileId,
                                                  DownloadUsage const usage )
{
    QString const fileName = tileFileName( tileId );
    QImage const image( fileName );
    if ( !image.isNull() && image.size() == m_textureLayer->tileSize() ) {
        // file is there, so create and return a tile object in any case,
        // but check if an update should be triggered
        QSharedPointer<TextureTile> const tile( new TextureTile( tileId, new QImage( image ), m_textureLayer ));
        tile->setStackedTileId( stackedTileId );
        tile->setLastModified( QFileInfo( fileName ).lastModified() );
        tile->setExpireSecs( m_textureLayer->expire() );

        if ( !tile->isExpired() ) {
            mDebug() << "TileLoader::loadTile" << tileId.toString() << "StateUptodate";
        } else {
            mDebug() << "TileLoader::loadTile" << tileId.toString() << "StateExpired";
            triggerDownload( tile, usage );
        }
        return tile;
    }

    // tile was not locally available => trigger download and look for tiles in other levels
    // for scaling
    QImage * replacementTile = scaledLowerLevelTile( tileId );
    QSharedPointer<TextureTile> const tile( new TextureTile( tileId, replacementTile, m_textureLayer ));
    tile->setStackedTileId( stackedTileId );

    triggerDownload( tile, usage );

    return tile;
}

// This method triggers a download of the given tile (without checking
// expiration). It is called by upper layer (StackedTileLoader) when the tile
// that should be reloaded is currently loaded in memory.
//
// post condition
//     - download is triggered
void TileLoader::reloadTile( QSharedPointer<TextureTile> const & tile, DownloadUsage const usage )
{
    triggerDownload( tile, usage );
}

void TileLoader::downloadTile( TileId const & id )
{
    QUrl const url = m_textureLayer->downloadUrl( id );
    QString const destFileName = m_textureLayer->relativeTileFileName( id );
    m_downloadManager->addJob( url, destFileName, id.toString(), DownloadBulk );
}

QString TileLoader::sourceDir() const
{
    return m_textureLayer->sourceDir();
}

Blending const * TileLoader::blending() const
{
    return m_textureLayer->blending();
}

TileLoader::Job::Job( QSharedPointer< TextureTile > tile, QObject* parent )
    : HttpDownloadJob( parent )
    , m_tile( tile )
{
}

void TileLoader::Job::execute( QByteArray const & data )
{
    QImage *image( new QImage( QImage::fromData( data ) ) );
    if ( image->isNull() )
        return;

    m_tile->setImage( image );
    m_tile->setLastModified( QDateTime::currentDateTime() );
    emit tileCompleted( m_tile->stackedTileId(), m_tile->id() );
}

QString TileLoader::tileFileName( TileId const & tileId ) const
{
    return MarbleDirs::path( m_textureLayer->relativeTileFileName( tileId ));
}

void TileLoader::triggerDownload( QSharedPointer<TextureTile> tile, DownloadUsage const usage )
{
    QUrl const sourceUrl = m_textureLayer->downloadUrl( tile->id() );
    QString const destFileName = m_textureLayer->relativeTileFileName( tile->id() );
    Job * job = new Job( tile );
    connect( job, SIGNAL(tileCompleted(TileId,TileId)), this, SIGNAL(tileCompleted(TileId,TileId)));
    m_downloadManager->addJob( sourceUrl, destFileName, job, usage );
}

    // TODO: get lastModified time stamp into the TextureTile
QImage * TileLoader::scaledLowerLevelTile( TileId const & id )
{
    mDebug() << "TileLoader::scaledLowerLevelTile" << id.toString();

    for ( int level = id.zoomLevel() - 1; level >= 0; --level ) {
        int const deltaLevel = id.zoomLevel() - level;
        TileId const replacementTileId( id.mapThemeIdHash(), level,
                                        id.x() >> deltaLevel, id.y() >> deltaLevel );
        mDebug() << "TileLoader::scaledLowerLevelTile" << "trying" << replacementTileId.toString();
        QString const fileName = tileFileName( replacementTileId );
        QImage const toScale( fileName );
        if ( !toScale.isNull() && toScale.size() == m_textureLayer->tileSize()  ) {
            // which rect to scale?
            QSize const size = toScale.size();
            int const restTileX = id.x() % ( 1 << deltaLevel );
            int const restTileY = id.y() % ( 1 << deltaLevel );
            int const partWidth = toScale.width() >> deltaLevel;
            int const partHeight = toScale.height() >> deltaLevel;
            int const startX = restTileX * partWidth;
            int const startY = restTileY * partHeight;
            mDebug() << "QImage::copy:" << startX << startY << partWidth << partHeight;
            QImage const part = toScale.copy( startX, startY, partWidth, partHeight );
            mDebug() << "QImage::scaled:" << toScale.size();
            return new QImage( part.scaled( toScale.size() ) );
        }
    }

    Q_ASSERT_X( false, "scaled image", "level zero image missing" ); // not reached
    return new QImage();
}

}

#include "TileLoader.moc"
