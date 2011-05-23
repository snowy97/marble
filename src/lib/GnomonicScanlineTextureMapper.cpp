//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Carlos Licea     <carlos _licea@hotmail.com>
// Copyright 2008      Inge Wallin      <inge@lysator.liu.se>
// Copyright 2011      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//


// local
#include "GnomonicScanlineTextureMapper.h"

// Qt
#include <QtCore/qmath.h>
#include <QtCore/QRunnable>
#include <QtGui/QImage>

// Marble
#include "GeoPainter.h"
#include "MarbleDirs.h"
#include "MarbleDebug.h"
#include "ScanlineTextureMapperContext.h"
#include "StackedTileLoader.h"
#include "TextureColorizer.h"
#include "ViewParams.h"
#include "ViewportParams.h"
#include "MathHelper.h"

using namespace Marble;

class GnomonicScanlineTextureMapper::RenderJob : public QRunnable
{
public:
    RenderJob( StackedTileLoader *tileLoader, int tileLevel, ViewParams *viewParams, int yTop, int yBottom );

    virtual void run();

private:
    StackedTileLoader *const m_tileLoader;
    const int m_tileLevel;
    QSharedPointer<QImage> m_canvasImage;
    ViewParams *const m_viewParams;
    const int m_yPaintedTop;
    const int m_yPaintedBottom;
};

GnomonicScanlineTextureMapper::RenderJob::RenderJob( StackedTileLoader *tileLoader, int tileLevel, ViewParams *viewParams, int yTop, int yBottom )
    : m_tileLoader( tileLoader ),
      m_tileLevel( tileLevel ),
      m_canvasImage( viewParams->canvasImagePtr() ),
      m_viewParams( viewParams ),
      m_yPaintedTop( yTop ),
      m_yPaintedBottom( yBottom )
{
}


GnomonicScanlineTextureMapper::GnomonicScanlineTextureMapper( StackedTileLoader *tileLoader,
                                                              QObject *parent )
    : TextureMapperInterface( parent )
    , m_tileLoader( tileLoader )
    , m_repaintNeeded( true )
{
}


void GnomonicScanlineTextureMapper::mapTexture( GeoPainter *painter,
                                                ViewParams *viewParams,
                                                const QRect &dirtyRect,
                                                TextureColorizer *texColorizer )
{
    if ( m_repaintNeeded ) {
        mapTexture( viewParams );

        if ( texColorizer ) {
            texColorizer->colorize( viewParams );
        }

        m_repaintNeeded = false;
    }

    painter->drawImage( dirtyRect, *viewParams->canvasImage(), dirtyRect );
}

void GnomonicScanlineTextureMapper::setRepaintNeeded()
{
    m_repaintNeeded = true;
}

void GnomonicScanlineTextureMapper::mapTexture( ViewParams *viewParams )
{
    // Reset backend
    m_tileLoader->resetTilehash();

    const int imageHeight = viewParams->canvasImagePtr()->height();

    const int numThreads = m_threadPool.maxThreadCount();
    const int yStep = imageHeight / numThreads;
    for ( int i = 0; i < numThreads; ++i ) {
        const int yStart =  i      * yStep;
        const int yEnd   = (i + 1) * yStep;
        QRunnable *const job = new RenderJob( m_tileLoader, tileZoomLevel(), viewParams, yStart, yEnd );
        m_threadPool.start( job );
    }

    m_threadPool.waitForDone();

    m_tileLoader->cleanupTilehash();
}

void GnomonicScanlineTextureMapper::RenderJob::run()
{
    QSharedPointer<QImage> canvasImage = m_viewParams->canvasImagePtr();

    const int imageHeight = canvasImage->height();
    const int imageWidth  = canvasImage->width();
    const qint64  radius  = m_viewParams->radius();
    // Calculate how many degrees are being represented per pixel.
    const qreal rad2Pixel = ( 2 * radius ) / M_PI;
    const qreal pixel2Rad = 1.0/rad2Pixel;

    const bool interlaced   = ( m_viewParams->mapQuality() == LowQuality );
    const bool highQuality  = ( m_viewParams->mapQuality() == HighQuality
                             || m_viewParams->mapQuality() == PrintQuality );
    const bool printQuality = ( m_viewParams->mapQuality() == PrintQuality );

    // Evaluate the degree of interpolation
    const int n = ScanlineTextureMapperContext::interpolationStep( m_viewParams );
    const int maxInterpolationPointX = n * (int)( imageWidth / n - 1 ) + 1; 

    // Calculate translation of center point
    qreal centerLon, centerLat;
    m_viewParams->centerCoordinates( centerLon, centerLat );


    // initialize needed variables that are modified during texture mapping:

    ScanlineTextureMapperContext context( m_tileLoader, m_tileLevel );


    // Paint the map.
    for ( int y = m_yPaintedTop; y < m_yPaintedBottom; ++y ) {

        QRgb * scanLine = (QRgb*)canvasImage->scanLine( y );

        for ( int x = 0; x < imageWidth; ++x ) {

            // Prepare for interpolation
            bool interpolate = false;
            if ( x >= 1 && x <= maxInterpolationPointX ) {
                x += n - 1;
                interpolate = !printQuality;
            }
            else {
                interpolate = false;
            }

            const qreal rx = ( - imageWidth  / 2 + x ) * pixel2Rad;
            const qreal ry = (   imageHeight / 2 - y ) * pixel2Rad;
            const qreal p = qSqrt( rx*rx + ry*ry );
            const qreal c = qAtan( p );
            qreal lon = centerLon + qAtan( rx*sin(c) / ( p*qCos( centerLat )*qCos( c ) - ry*qSin( centerLat )*qSin( c )  ) );
            while ( lon < -M_PI ) lon += 2 * M_PI;
            while ( lon >  M_PI ) lon -= 2 * M_PI;

            const qreal lat = qAsin( qCos(c)*qSin(centerLat) + ry*qSin(c)*qCos(centerLat)/p );
            if ( lon < -M_PI ) lon += 2 * M_PI;
            if ( lon >  M_PI ) lon -= 2 * M_PI;

            if ( interpolate ) {
                if ( highQuality )
                    context.pixelValueApproxF( lon, lat, scanLine, n );
                else
                    context.pixelValueApprox( lon, lat, scanLine, n );

                scanLine += ( n - 1 );
            }

            if ( x < imageWidth ) {
                if ( highQuality )
                    context.pixelValueF( lon, lat, scanLine );
                else
                    context.pixelValue( lon, lat, scanLine );
            }

            ++scanLine;
            lon += pixel2Rad;
        }

        // copy scanline to improve performance
        if ( interlaced && y + 1 < m_yPaintedBottom ) { 

            const int pixelByteSize = canvasImage->bytesPerLine() / imageWidth;

            memcpy( canvasImage->scanLine( y + 1 ),
                    canvasImage->scanLine( y     ),
                    imageWidth * pixelByteSize );
            ++y;
        }
    }
}


#include "GnomonicScanlineTextureMapper.moc"
