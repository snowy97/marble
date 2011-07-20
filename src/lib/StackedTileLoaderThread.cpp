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


#include "StackedTileLoaderThread.h"

#include "MergedLayerDecorator.h"
#include "TextureTile.h"

#include <QtCore/QMutexLocker>
#include <QtCore/QPointer>
#include <QtCore/QRunnable>
#include <QtGui/QPainter>

namespace Marble
{

class StackedTileLoaderThread::MergeJob : public QRunnable
{
public:
    MergeJob( const TileId &id, const QVector<TextureTile> &tiles, StackedTileLoaderThread *parent )
        : m_id( id ),
          m_tiles( tiles ),
          m_parent( parent )
    {}

    virtual void run()
    {
        m_parent->merge( m_id, m_tiles );
    }

private:
    const TileId m_id;
    const QVector<TextureTile> m_tiles;
    StackedTileLoaderThread *const m_parent;
};

class StackedTileLoaderThread::TileJob : public QRunnable
{
public:
    TileJob( const TileId &id, const QVector<const GeoSceneTexture *> &textureLayers, StackedTileLoaderThread *parent )
        : m_id( id ),
          m_textureLayers( textureLayers ),
          m_parent( parent )
    {}

    virtual void run()
    {
        m_parent->create( m_id, m_textureLayers );
    }

private:
    const TileId m_id;
    const QVector<const GeoSceneTexture *> m_textureLayers;
    StackedTileLoaderThread *const m_parent;
};

StackedTileLoaderThread::StackedTileLoaderThread( MergedLayerDecorator *decorator )
    : m_decorator( decorator ),
      m_run( true )
{
}

StackedTileLoaderThread::~StackedTileLoaderThread()
{
    m_run = false;
    wait();
}

void StackedTileLoaderThread::run()
{
    while ( m_run ) {
        m_queueMutex.lock();
        QRunnable *const job = !m_highPriorityTileQueue.isEmpty() ? m_highPriorityTileQueue.take( m_highPriorityTileQueue.keys().first() ) :
                               !m_highPriorityMergeQueue.isEmpty() ? m_highPriorityMergeQueue.take( m_highPriorityMergeQueue.keys().first() ) :
                               !m_mergeQueue.isEmpty() ? m_mergeQueue.take( m_mergeQueue.keys().first() ) :
                               !m_tileQueue.isEmpty() ? m_tileQueue.takeFirst() : 0;
        m_queueMutex.unlock();

        if ( job == 0 ) {
            break;
        }

        job->run();
        delete job;
    };
}

void StackedTileLoaderThread::merge( const TileId &id, const QVector<TextureTile> &tiles )
{
    StackedTile tile = m_decorator->merge( id, tiles );

    emit tileFinished( id, tile, tiles );
}

void StackedTileLoaderThread::create( const TileId &id, const QVector<const GeoSceneTexture *> &textureLayers )
{
    QVector<TextureTile> tiles = m_decorator->createTile( id, textureLayers );
    StackedTile tile = m_decorator->merge( id, tiles );

    emit tileFinished( id, tile, tiles );
}

void StackedTileLoaderThread::requestTile(const TileId &stackedTileId, const QVector<const GeoSceneTexture *> &textureLayers, bool highPriority )
{
    QRunnable *job = new TileJob( stackedTileId, textureLayers, this );

    QMutexLocker locker( &m_queueMutex );
    if ( highPriority ) {
        delete m_highPriorityTileQueue.value( stackedTileId, 0 );
        m_highPriorityTileQueue.insert( stackedTileId, job );
    }
    else
        m_tileQueue.append( job );
    if (!isRunning())
        start();
}

void StackedTileLoaderThread::clear()
{
    QMutexLocker locker( &m_queueMutex );
    qDeleteAll( m_tileQueue );
    m_tileQueue.clear();
}

void StackedTileLoaderThread::requestMerge( const QVector<TextureTile> &tiles, bool highPriority )
{
    if ( tiles.isEmpty() )
        return;

    const TileId id = TileId( 0,
                              tiles.first().id().zoomLevel(),
                              tiles.first().id().x(),
                              tiles.first().id().y() );

    MergeJob * const job = new MergeJob( id, tiles, this );

    QMutexLocker locker( &m_queueMutex );
    if ( highPriority ) {
        delete m_highPriorityMergeQueue.value( id, 0 );
        m_highPriorityMergeQueue.insert( id, job );
    } else {
        delete m_mergeQueue.value( id, 0 );
        m_mergeQueue.insert( id, job );
    }
    if (!isRunning())
        start();
}

}

#include "StackedTileLoaderThread.moc"
