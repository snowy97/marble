//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011       Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>

#include "layers/TextureLayer.h"

#include "MarbleModel.h"

#define addRow() QTest::newRow( QString("line %1").arg( __LINE__ ).toAscii().data() )

namespace Marble
{

class TextureLayerTest : public QObject
{
    Q_OBJECT

 private slots:
    void constructor();
};

void TextureLayerTest::constructor()
{
    MarbleModel model;

    TextureLayer layer( model.downloadManager(), model.sunLocator() );
}

}

QTEST_MAIN( Marble::TextureLayerTest )

#include "TextureLayerTest.moc"
