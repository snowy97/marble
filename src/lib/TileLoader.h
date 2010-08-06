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

#ifndef MARBLE_TILELOADER_H
#define MARBLE_TILELOADER_H

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>

#include "TileId.h"
#include "global.h"
#include "HttpDownloadManager.h"

class QByteArray;
class QImage;
class QUrl;

namespace Marble
{

class Blending;
class HttpDownloadManager; // remove?
class GeoSceneTexture;
class TextureTile;

class TileLoader: public QObject
{
    Q_OBJECT

 public:
    TileLoader( GeoSceneTexture const * textureLayer, HttpDownloadManager * const );

    QSharedPointer<TextureTile> loadTile( TileId const & stackedTileId, TileId const & tileId,
                                          DownloadUsage const );
    void reloadTile( QSharedPointer<TextureTile> const & tile, DownloadUsage const );
    void downloadTile( TileId const & tileId );

    QString sourceDir() const;
    Blending const * blending() const;

 Q_SIGNALS:
    // when this signal is emitted, the TileLoader gives up ownership of
    // the corrsponding tile. Might be better to explicitly transfer...
    void tileCompleted( TileId const & composedTileId, TileId const & baseTileId );

 private:
    QString tileFileName( TileId const & ) const;
    void triggerDownload( QSharedPointer<TextureTile> tile, DownloadUsage const );
    QImage * scaledLowerLevelTile( TileId const & );

    GeoSceneTexture const * m_textureLayer;

    HttpDownloadManager * const m_downloadManager;

    class Job;
};

class TileLoader::Job : public HttpDownloadJob
{
    Q_OBJECT

public:
    Job( QSharedPointer<TextureTile> tile, QObject * parent = 0 );

public Q_SLOTS:
    virtual void execute( QByteArray const & data );

Q_SIGNALS:
    void tileCompleted( TileId const & composedTileId, TileId const & baseTileId );

private:
    QSharedPointer<TextureTile> m_tile;
};

}

#endif
