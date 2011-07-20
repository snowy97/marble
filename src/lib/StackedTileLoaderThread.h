// Copyright 2011 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public 
// License along with this library.  If not, see <http://www.gnu.org/licenses/>.

#ifndef MARBLE_STACKEDTILELOADERTHREAD_H
#define MARBLE_STACKEDTILELOADERTHREAD_H

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QQueue>

#include "StackedTile.h"
#include "TextureTile.h"
#include "TileId.h"
#include "global.h"

class QRunnable;

namespace Marble
{
class GeoSceneTexture;
class MergedLayerDecorator;
class TileId;

class StackedTileLoaderThread : public QThread
{
    Q_OBJECT

 public:
    StackedTileLoaderThread( MergedLayerDecorator *decorator );
    virtual ~StackedTileLoaderThread();

 public Q_SLOTS:
    void requestMerge( const QVector<TextureTile> &tiles, bool highPriority );
    void requestTile( const TileId &stackedTileId, const QVector<const GeoSceneTexture *> &textureLayers, bool highPriority );
    void clear();

 Q_SIGNALS:
    void tileFinished( const TileId &id, const StackedTile &tile, const QVector<TextureTile> & );

 protected:
    virtual void run();

 private:
    class MergeJob;
    class TileJob;

    void merge( const TileId &id, const QVector<TextureTile> &tiles );
    void create( const TileId &id, const QVector<const GeoSceneTexture *> &textureLayers );

 protected:
    Q_DISABLE_COPY( StackedTileLoaderThread )
    MergedLayerDecorator *const m_decorator;
    QMutex m_queueMutex;
    QQueue<QRunnable *> m_tileQueue;
    QHash<TileId, QRunnable *> m_highPriorityTileQueue;
    QHash<TileId, QRunnable *> m_mergeQueue;
    QHash<TileId, QRunnable *> m_highPriorityMergeQueue;
    bool m_run;
};

}

#endif
