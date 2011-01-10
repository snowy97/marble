//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_GNOMONICSCANLINETEXTUREMAPPER_H
#define MARBLE_GNOMONICSCANLINETEXTUREMAPPER_H


#include "TextureMapperInterface.h"

namespace Marble
{

class StackedTileLoader;

class GnomonicScanlineTextureMapper : public TextureMapperInterface
{
    Q_OBJECT

 public:
    GnomonicScanlineTextureMapper( StackedTileLoader *tileLoader, QObject *parent = 0 );

    virtual void mapTexture( GeoPainter *painter,
                             ViewParams *viewParams,
                             const QRect &dirtyRect,
                             TextureColorizer *texColorizer );

    virtual void setRepaintNeeded();

 private:
    void mapTexture( ViewParams *viewParams );

    StackedTileLoader *const m_tileLoader;
    bool m_repaintNeeded;
};

}

#endif
