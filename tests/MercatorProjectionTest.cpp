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
#include "MercatorProjection.h"
#include "ViewportParams.h"

namespace QTest
{

bool qCompare(qreal val1, qreal val2, qreal epsilon, const char *actual, const char *expected, const char *file, int line)
{
    return ( qAbs( val1 - val2 ) < epsilon )
        ? compare_helper( true, "COMPARE()", file, line )
        : compare_helper( false, "Compared qreals are not the same", toString( val1 ), toString( val2 ), actual, expected, file, line );
}

}

#define QFUZZYCOMPARE(actual, expected, epsilon) \
do {\
    if (!QTest::qCompare(actual, expected, epsilon, #actual, #expected, __FILE__, __LINE__))\
        return;\
} while (0)

#define addRow() QTest::newRow( QString("line %1").arg( __LINE__ ).toAscii().data() )

namespace Marble
{

class MercatorProjectionTest : public QObject
{
    Q_OBJECT

 private slots:
    void screenCoordinates_data();
    void screenCoordinates();
};

void MercatorProjectionTest::screenCoordinates_data()
{
    QTest::addColumn<qreal>( "lon" );
    QTest::addColumn<qreal>( "lat" );

    addRow() << 0.0 << 0.0;

    addRow() << -179.99 << 0.0;
    addRow() <<  179.99 << 0.0;
}

void MercatorProjectionTest::screenCoordinates()
{
    QFETCH( qreal, lon );
    QFETCH( qreal, lat );

    const GeoDataCoordinates coordinates( lon, lat, 0, GeoDataCoordinates::Degree );

    ViewportParams viewport;
    viewport.setProjection( Mercator );
    viewport.setRadius( 360 / 4 ); // for easy mapping of lon <-> x
    viewport.centerOn( 0, 0 );
    viewport.setSize( QSize( 360, 360 ) );

    {
        qreal x = 0.1234;
        qreal y = 5.6789;

        const bool retval = viewport.currentProjection()->screenCoordinates( lon * DEG2RAD, lat * DEG2RAD, &viewport, x, y );

        QVERIFY( retval );
        QCOMPARE( x, lon + 180.0 );
        QCOMPARE( y, lat + 180.0 );
    }

    {
        qreal x = 0.1234;
        qreal y = 5.6789;
        bool globeHidesPoint = true;

        const bool retval = viewport.currentProjection()->screenCoordinates( coordinates, &viewport, x, y, globeHidesPoint );

        QVERIFY( retval );
        QCOMPARE( x, lon + 180.0 );
        QCOMPARE( y, lat + 180.0 );
        QVERIFY( !globeHidesPoint );
    }

    QVERIFY( viewport.currentProjection()->repeatX() );

    {
        qreal x[1] = { 0.1234 };
        qreal y = 5.6789;
        int pointRepeatNum = 1000;
        bool globeHidesPoint = true;

        const bool retval = viewport.currentProjection()->screenCoordinates( coordinates, &viewport, x, y, pointRepeatNum, QSizeF( 0, 0 ), globeHidesPoint );

        QVERIFY( retval );
        QCOMPARE( pointRepeatNum, 1 );
        QCOMPARE( x[0], lon + 180.0 );
        QCOMPARE( y, lat + 180.0 );
        QVERIFY( !globeHidesPoint );
    }
}

}

QTEST_MAIN( Marble::MercatorProjectionTest )

#include "MercatorProjectionTest.moc"
